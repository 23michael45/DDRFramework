#ifndef AudioCodec_h__
#define AudioCodec_h__

#include <stdio.h>

#ifdef DR_FLAC_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/extras/dr_flac.h"  // Enables FLAC decoding.
#endif
#ifdef DR_MP3_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/extras/dr_mp3.h"   // Enables MP3 decoding.
#endif
#ifdef DR_WAV_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/extras/dr_wav.h"   // Enables WAV decoding.
#endif
#ifdef MINI_AL_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/mini_al.h"
#endif


namespace DDRFramework
{
	class AudioCodec
	{
	public:
		AudioCodec();
		~AudioCodec();

		bool Init(int channelCount, int sampleRate, mal_recv_proc recv, mal_send_proc send);
		void Deinit();

		bool StartRecord();
		void StopRecord();
		bool StartPlay();
		void StopPlay();


	private:



		mal_context m_Context;
		mal_device_config m_Config;
		mal_device m_CaptureDevice;
		mal_device m_PlaybackDevice;


		mal_uint32 capturedSampleCount = 0;
		mal_int16* pCapturedSamples = NULL;
		mal_uint32 playbackSample = 0;
	};


}

#endif // AudioCodec_h__
