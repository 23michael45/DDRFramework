#include "AudioCodec.h"
#include <memory>
#include <fstream>
#include "../../thirdparty/asio/include/asio.hpp"
#include <functional>

#include "../../../Shared/src/Utility/Logger.h"
#include "../../../Shared/src/Utility/DDRMacro.h"

namespace DDRFramework
{


	void AudioCodec::on_recv_frames_device(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples)
	{
		mal_uint32 sampleCount = frameCount * pDevice->channels;

		std::shared_ptr<asio::streambuf> buf = std::make_shared<asio::streambuf>();

		std::ostream oshold(buf.get());
		oshold.write((const char*)pSamples, sampleCount * sizeof(mal_int16));
		oshold.flush();

		if (m_spSession)
		{
			m_spSession->Send(buf);
		}

	}

	mal_uint32 AudioCodec::on_send_frames_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
	{
		std::lock_guard<std::mutex> lock(m_AudioRecvMutex);
		mal_uint32 samplesToRead = frameCount * pDevice->channels;

		asio::streambuf* pbuf;
		std::shared_ptr<asio::streambuf> spbuf = nullptr;


		auto pair = GetQueueAudio();
		if (pair.first >= 0 && pair.second)//if exsit element to play
		{
			pbuf = pair.second.get();
		}
		else
		{
			pbuf = &m_AudioRecvBuf;
		}



		size_t len = pbuf->size();
		if (len < samplesToRead * sizeof(mal_int16))
		{

			memcpy(pSamples, pbuf->data().data(), len);
			pbuf->consume(len);

			EndPlay(pair);

			return len / sizeof(mal_int16) / pDevice->channels;
		}
		else
		{

			memcpy(pSamples, pbuf->data().data(), samplesToRead * sizeof(mal_int16));
			pbuf->consume(samplesToRead * sizeof(mal_int16));
			return samplesToRead / pDevice->channels;
		}


	}




	AudioCodec::AudioCodec()
	{
	}

	AudioCodec::~AudioCodec()
	{
	}



	bool AudioCodec::Init()
	{

		int channelCount = 1;
		int sampleRate = 16000;
		mal_recv_proc recv = std::bind(&AudioCodec::on_recv_frames_device, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		mal_send_proc send = std::bind(&AudioCodec::on_send_frames_device, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		

		if (mal_context_init(NULL, 0, NULL, &m_Context) != MAL_SUCCESS) {
			DebugLog("Failed to initialize audio context.");
			return -1;
		}

		m_Config = mal_device_config_init(mal_format_s16, channelCount, sampleRate, recv, send);
		
	}
	void AudioCodec::Deinit()
	{

		mal_context_uninit(&m_Context);

	}
	bool AudioCodec::StartRecord()
	{
		printf("Recording...\n");
		if (mal_device_init(&m_Context, mal_device_type_capture, NULL, &m_Config, NULL, &m_CaptureDevice) != MAL_SUCCESS) {
			mal_context_uninit(&m_Context);
			printf("Failed to initialize capture device.\n");
			return -2;
		}

		if (mal_device_start(&m_CaptureDevice) != MAL_SUCCESS) {
			mal_device_uninit(&m_CaptureDevice);
			mal_context_uninit(&m_Context);
			printf("Failed to start capture device.\n");
			return -3;
		}
		return true;
	}
	void AudioCodec::StopRecord()
	{
		mal_device_uninit(&m_CaptureDevice);
	}

	bool AudioCodec::StartPlay()
	{
		printf("Playing...\n");
		if (mal_device_init(&m_Context, mal_device_type_playback, NULL, &m_Config, NULL, &m_PlaybackDevice) != MAL_SUCCESS) {
			mal_context_uninit(&m_Context);
			printf("Failed to initialize playback device.\n");
			return -4;
		}

		if (mal_device_start(&m_PlaybackDevice) != MAL_SUCCESS) {
			mal_device_uninit(&m_PlaybackDevice);
			mal_context_uninit(&m_Context);
			printf("Failed to start playback device.\n");
			return -5;
		}

		return true;

	}
	void AudioCodec::StopPlay()
	{
		mal_device_uninit(&m_PlaybackDevice);
	}

	mal_uint32 AudioCodec::on_send_frames_to_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
	{
		mal_decoder* pDecoder = (mal_decoder*)pDevice->pUserData;
		if (pDecoder == NULL) {
			return 0;
		}

		return (mal_uint32)mal_decoder_read(pDecoder, frameCount, pSamples);
	}
	void AudioCodec::StartPlayFile(asio::streambuf& buf)
	{
		mal_result result = mal_decoder_init_memory_wav(buf.data().data(),buf.size(), (const mal_decoder_config*)&m_FileConfig, &m_FileDecoder);
		if (result != MAL_SUCCESS) {
			return;
		}


	}
	void AudioCodec::StartPlayFile(std::string fileName)
	{
		mal_result result = mal_decoder_init_file(fileName.c_str(), NULL, &m_FileDecoder);
		if (result != MAL_SUCCESS) {
			return;
		}

		m_FileConfig = mal_device_config_init_playback(m_FileDecoder.outputFormat,m_FileDecoder.outputChannels,m_FileDecoder.outputSampleRate,std::bind(&AudioCodec::on_send_frames_to_device, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		mal_device device;
		if (mal_device_init(NULL, mal_device_type_playback, NULL, &m_FileConfig, &m_FileDecoder, &m_PlayFileDevice) != MAL_SUCCESS) {
			printf("Failed to open playback device.\n");
			mal_decoder_uninit(&m_FileDecoder);
			return;
		}

		if (mal_device_start(&m_PlayFileDevice) != MAL_SUCCESS) {
			printf("Failed to start playback device.\n");
			mal_device_uninit(&m_PlayFileDevice);
			mal_decoder_uninit(&m_FileDecoder);
			return;
		}

	}

	void AudioCodec::StopPlayFile()
	{
		mal_device_uninit(&m_PlayFileDevice);
		mal_decoder_uninit(&m_FileDecoder);

	}



	void AudioCodec::PlayAudio(std::shared_ptr<asio::streambuf> spbuf, int priority)
	{
		std::lock_guard<std::mutex> lock(m_AudioQueueMutex);
		if (m_AudioQueueMap.find(priority) == m_AudioQueueMap.end())
		{
			auto spQueue = std::make_shared<std::queue<std::shared_ptr<asio::streambuf>>>();
			m_AudioQueueMap.insert(make_pair(priority, spQueue));
		}

		if (spbuf)
		{
			auto spQueue = m_AudioQueueMap[priority];
			spQueue->push(spbuf);

		}
	}

	void AudioCodec::EndPlay(std::pair<int, std::shared_ptr<asio::streambuf>> pair)
	{
		std::lock_guard<std::mutex> lock(m_AudioQueueMutex);
		if (m_AudioQueueMap.find(pair.first) != m_AudioQueueMap.end())
		{
			m_AudioQueueMap[pair.first]->pop();
		}
	}

	std::pair<int, std::shared_ptr<asio::streambuf>> AudioCodec::GetQueueAudio()
	{
		std::lock_guard<std::mutex> lock(m_AudioQueueMutex);
		std::shared_ptr<asio::streambuf> spbuf;
		for (auto spQueuePair : m_AudioQueueMap)
		{
			auto spQueue = spQueuePair.second;
			if (spQueue->size() > 0)
			{
				spbuf = spQueue->front();
				spQueue->pop();

				return make_pair(spQueuePair.first, spbuf);
			}
		}
		return std::make_pair(-1, nullptr);
	}

}