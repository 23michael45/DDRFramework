#include "AudioCodec.h"
#include <memory>
#include <fstream>
#include "../../thirdparty/asio/include/asio.hpp"



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
}