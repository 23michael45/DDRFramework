#include "AudioCodec.h"
#include <memory>



namespace DDRFramework
{

	AudioCodec::AudioCodec()
	{
	}

	AudioCodec::~AudioCodec()
	{
	}
	void on_recv_frames(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples)
	{
		/*mal_uint32 sampleCount = frameCount * pDevice->channels;

		mal_uint32 newCapturedSampleCount = capturedSampleCount + sampleCount;
		mal_int16* pNewCapturedSamples = (mal_int16*)realloc(pCapturedSamples, newCapturedSampleCount * sizeof(mal_int16));
		if (pNewCapturedSamples == NULL) {
			return;
		}

		memcpy(pNewCapturedSamples + capturedSampleCount, pSamples, sampleCount * sizeof(mal_int16));

		pCapturedSamples = pNewCapturedSamples;
		capturedSampleCount = newCapturedSampleCount;*/
	}

	mal_uint32 on_send_frames(mal_device* pDevice, mal_uint32 frameCount, void* pSamples)
	{
		//mal_uint32 samplesToRead = frameCount * pDevice->channels;
		//if (samplesToRead > capturedSampleCount - playbackSample) {
		//	samplesToRead = capturedSampleCount - playbackSample;
		//}

		//if (samplesToRead == 0) {
		//	return 0;
		//}

		//memcpy(pSamples, pCapturedSamples + playbackSample, samplesToRead * sizeof(mal_int16));
		//playbackSample += samplesToRead;

		//return samplesToRead / pDevice->channels;
		return 0;
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