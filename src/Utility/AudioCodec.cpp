#include "AudioCodec.h"
#include <memory>
#include <fstream>
#include "../../thirdparty/asio/include/asio.hpp"
#include <functional>


namespace DDRFramework
{

	AudioCodec::AudioCodec()
	{
	}

	AudioCodec::~AudioCodec()
	{
	}



	bool AudioCodec::Init(int channelCount, int sampleRate,mal_recv_proc recv, mal_send_proc send)
	{
		if (mal_context_init(NULL, 0, NULL, &m_Context) != MAL_SUCCESS) {
			printf("Failed to initialize context.");
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

}