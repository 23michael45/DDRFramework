#ifndef AudioCodec_h__
#define AudioCodec_h__

#include <stdio.h>

#ifdef MINI_AL_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/mini_al.h"
#endif

#include "../../src/Network/TcpSocketContainer.h"

#include <queue>
#include <stack>

namespace DDRFramework
{
	struct WavBufInfo
	{
		std::shared_ptr<asio::streambuf> m_spBuf;
		int m_CurrentFrame;
		int m_Priority;
	};

	class AudioCodec
	{
	public:
		AudioCodec();
		~AudioCodec();

		bool Init();
		void Deinit();

		bool StartDeviceRecord();
		void StopDeviceRecord();
		bool StartDevicePlay();
		void StopDevicePlay();


		void PushAudioRecvBuf(asio::streambuf& buf);



		void on_recv_frames_device(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples);
		mal_uint32 on_send_frames_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);
		mal_uint32 on_send_frames_wavfile(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);


		bool StartPlayBuf(std::shared_ptr<WavBufInfo> spInfo,int frameCount = 0);
		bool StartPlayFile(std::string fileName);
		
		//return current framecount
		int StopPlayBuf();



		void SetTcpSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			m_spSession = sp;
		}
		void BindOnFinishPlayWave(std::function<void(std::shared_ptr<WavBufInfo>)> func)
		{
			m_OnFinishPlayWavFunc = func;

		}

	private:

		mal_context m_Context;
		mal_device_config m_Config;
		mal_device m_CaptureDevice;
		mal_device m_PlaybackDevice;


		mal_device_config m_FileConfig;
		mal_device m_PlayFileDevice;
		mal_decoder m_FileDecoder;


		mal_uint32 capturedSampleCount = 0;
		mal_int16* pCapturedSamples = NULL;
		mal_uint32 playbackSample = 0;

		std::shared_ptr<TcpSocketContainer> m_spSession;

		std::function<void(std::shared_ptr<WavBufInfo>)> m_OnFinishPlayWavFunc;
		std::shared_ptr<WavBufInfo> m_spCurrentWavBufInfo;


		asio::streambuf m_AudioRecvBuf;

		std::mutex m_AudioRecvMutex;


	};


}

#endif // AudioCodec_h__
