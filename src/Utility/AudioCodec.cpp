#include "AudioCodec.h"
#include <memory>
#include <fstream>
#include "../../thirdparty/asio/include/asio.hpp"
#include <functional>

#include "../../../Shared/src/Utility/Logger.h"
#include "../../../Shared/src/Utility/DDRMacro.h"

namespace DDRFramework
{

	//sound wav from mic to client
	void AudioCodec::on_recv_frames_device(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples)
	{
		mal_uint32 sampleCount = frameCount * pDevice->channels;

		std::shared_ptr<asio::streambuf> buf = std::make_shared<asio::streambuf>();

		if (pSamples != nullptr && sampleCount > 0)
		{


			std::ostream oshold(buf.get());
			oshold.write((const char*)pSamples, sampleCount * sizeof(mal_int16));
			oshold.flush();

			//DebugLog("on_recv_frames_device %d", sampleCount);

			std::lock_guard<std::mutex> lock(m_AudioSendMutex);

			for (auto spSessionSendTo : m_spSessionSendToSet)
			{
				spSessionSendTo->Send(buf);
			}
		}

	}

	//sound wav from  client to speaker
	mal_uint32 AudioCodec::on_send_frames_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
	{
		std::lock_guard<std::mutex> lock(m_AudioRecvMutex);
		mal_uint32 samplesToRead = frameCount * pDevice->channels;

		asio::streambuf* pbuf;
		pbuf = &m_AudioRecvBuf;

		size_t len = pbuf->size();
		if (len < samplesToRead * sizeof(mal_int16))
		{

			memcpy(pSamples, pbuf->data().data(), len);
			pbuf->consume(len);

			return len / sizeof(mal_int16) / pDevice->channels;
		}
		else
		{

			memcpy(pSamples, pbuf->data().data(), samplesToRead * sizeof(mal_int16));
			pbuf->consume(samplesToRead * sizeof(mal_int16));
			return samplesToRead / pDevice->channels;
		}


	}

	mal_uint32 AudioCodec::on_send_frames_wavfile(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
	{
		if (m_spCurrentWavBufInfo)
		{
			m_spCurrentWavBufInfo->m_CurrentFrame += frameCount;
		}
		mal_decoder* pDecoder = (mal_decoder*)pDevice->pUserData;
		if (pDecoder == NULL) {
			return 0;
		}

		if (pDecoder->memory.currentReadPos >= pDecoder->memory.dataSize)
		{

			if (m_OnFinishPlayWavFunc)
			{
				std::thread t(m_OnFinishPlayWavFunc, m_spCurrentWavBufInfo);
				t.detach();
			}
		}


		return (mal_uint32)mal_decoder_read(pDecoder, frameCount, pSamples);
	}



	AudioCodec::AudioCodec()
	{
		m_pCaptureDevice = nullptr;
		m_pFileDecoder = nullptr;
		m_pPlaybackDevice = nullptr;
		m_pPlayFileDevice = nullptr;
		m_pContext = nullptr;
	}

	AudioCodec::~AudioCodec()
	{
		StopDevicePlay();
		StopDeviceRecord();
		StopPlayBuf();

		Deinit();
	}



	bool AudioCodec::Init()
	{

		int channelCount = 1;
		int sampleRate = 16000;
		mal_recv_proc recv = std::bind(&AudioCodec::on_recv_frames_device, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		mal_send_proc send = std::bind(&AudioCodec::on_send_frames_device, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		
		m_pContext = new mal_context();
		if (mal_context_init(NULL, 0, NULL, m_pContext) != MAL_SUCCESS) {
			DebugLog("Failed to initialize audio context.");

			SAFE_DELETE(m_pContext);
			return -1;
		}

		m_Config = mal_device_config_init(mal_format_s16, channelCount, sampleRate, recv, send);
		
	}
	void AudioCodec::Deinit()
	{
		if (m_pContext)
		{
			mal_context_uninit(m_pContext); 
			SAFE_DELETE(m_pContext);
		}


	}
	bool AudioCodec::StartDeviceRecord()
	{
		try
		{

			printf("Recording...\n");
			if (m_pCaptureDevice == NULL)
			{
				m_pCaptureDevice = new mal_device();
				if (mal_device_init(m_pContext, mal_device_type_capture, NULL, &m_Config, NULL, m_pCaptureDevice) != MAL_SUCCESS) {
					mal_context_uninit(m_pContext);
					SAFE_DELETE(m_pCaptureDevice);
					SAFE_DELETE(m_pContext);
					printf("Failed to initialize capture device.\n");
					return false;
				}
				if (mal_device_start(m_pCaptureDevice) != MAL_SUCCESS) {
					mal_device_uninit(m_pCaptureDevice);
					mal_context_uninit(m_pContext);
					SAFE_DELETE(m_pCaptureDevice);
					SAFE_DELETE(m_pContext);
					printf("Failed to start capture device.\n");
					return false;
				}
				return true;
			}
		}
		catch (std::exception& e)
		{

			return false;
		}
	}
	void AudioCodec::StopDeviceRecord()
	{
		if (m_pCaptureDevice)
		{
			mal_device_uninit(m_pCaptureDevice);
			SAFE_DELETE(m_pCaptureDevice);
		}
	}

	bool AudioCodec::StartDevicePlay()
	{
		try
		{

			printf("Playing...\n");
			if (m_pPlaybackDevice == NULL)
			{
				m_pPlaybackDevice = new mal_device();
				if (mal_device_init(m_pContext, mal_device_type_playback, NULL, &m_Config, NULL, m_pPlaybackDevice) != MAL_SUCCESS) {
					mal_context_uninit(m_pContext);

					SAFE_DELETE(m_pPlaybackDevice);
					SAFE_DELETE(m_pContext);
					printf("Failed to initialize playback device.\n");
					return false;
				}
				if (mal_device_start(m_pPlaybackDevice) != MAL_SUCCESS) {
					mal_device_uninit(m_pPlaybackDevice);
					mal_context_uninit(m_pContext);

					SAFE_DELETE(m_pPlaybackDevice);
					SAFE_DELETE(m_pContext);
					printf("Failed to start playback device.\n");
					return false;
				}

				return true;
			}
		}
		catch (std::exception& e)
		{

			return false;
		}

	}
	void AudioCodec::StopDevicePlay()
	{
		if (m_pPlaybackDevice)
		{
			mal_device_uninit(m_pPlaybackDevice);
			SAFE_DELETE(m_pPlaybackDevice);
		}
	}

	void AudioCodec::PushAudioRecvBuf(asio::streambuf& buf)
	{
		std::lock_guard<std::mutex> lock(m_AudioRecvMutex);
		std::ostream oshold(&m_AudioRecvBuf);
		//int size = buf.size();
		//DebugLog("PushAudioRecvBuf %i",size)

		oshold.write((const char*)buf.data().data(),size);
		oshold.flush();
	}

	bool AudioCodec::StartPlayBuf(std::shared_ptr<WavBufInfo> spInfo)
	{
		std::lock_guard<std::mutex> lock(m_AudioBufPlayMutex);

		if (m_spCurrentWavBufInfo)
		{
			return false;
		}
		m_spCurrentWavBufInfo = spInfo;
		auto spbuf = spInfo->m_spBuf;

		memset(&m_FileConfig, 0, sizeof(m_FileConfig));

		m_pFileDecoder = new mal_decoder();
		mal_result result = mal_decoder_init_memory_wav(spbuf->data().data(), spbuf->size(), (const mal_decoder_config*)&m_FileConfig, m_pFileDecoder);
		if (result != MAL_SUCCESS) {
			SAFE_DELETE(m_pFileDecoder);
			return false;
		}
		if (m_spCurrentWavBufInfo->m_CurrentFrame > 0)
		{
			mal_decoder_seek_to_frame(m_pFileDecoder, m_spCurrentWavBufInfo->m_CurrentFrame);
		}

		auto config = mal_device_config_init_playback(m_pFileDecoder->outputFormat, m_pFileDecoder->outputChannels, m_pFileDecoder->outputSampleRate, std::bind(&AudioCodec::on_send_frames_wavfile, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		m_pPlayFileDevice = new mal_device();
		if (mal_device_init(NULL, mal_device_type_playback, NULL, &config, m_pFileDecoder, m_pPlayFileDevice) != MAL_SUCCESS) {
			printf("Failed to open playback device.\n");
			mal_decoder_uninit(m_pFileDecoder);
			SAFE_DELETE(m_pFileDecoder);
			SAFE_DELETE(m_pPlayFileDevice);
			return false;
		}

		if (mal_device_start(m_pPlayFileDevice) != MAL_SUCCESS) {
			printf("Failed to start playback device.\n");
			mal_device_uninit(m_pPlayFileDevice);
			mal_decoder_uninit(m_pFileDecoder);
			SAFE_DELETE(m_pPlayFileDevice);
			SAFE_DELETE(m_pFileDecoder);
			return false;
		}

		return true;

	}

	std::shared_ptr<WavBufInfo>  AudioCodec::StopPlayBuf()
	{
		std::lock_guard<std::mutex> lock(m_AudioBufPlayMutex);

		if (m_pPlayFileDevice)
		{
			mal_device_uninit(m_pPlayFileDevice);
			SAFE_DELETE(m_pPlayFileDevice);
		}
		if (m_pFileDecoder)
		{
			mal_decoder_uninit(m_pFileDecoder);
			SAFE_DELETE(m_pFileDecoder);
		}


		auto sp = m_spCurrentWavBufInfo;
		m_spCurrentWavBufInfo.reset();
		return sp;
	}

}