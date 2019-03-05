#ifndef AudioCodec_h__
#define AudioCodec_h__

#include <stdio.h>

#ifdef MINI_AL_IMPLEMENTATION
#include "../../Shared/thirdparty/mini-al/mini_al.h"
#endif

#include "../../src/Network/TcpSocketContainer.h"
#include "../../src/Utility/DDRMacro.h"
#include "../../src/Utility/Logger.h"
#include <queue>
#include <stack>

namespace DDRFramework
{
	struct WavBufInfo
	{
		~WavBufInfo()
		{
			DebugLog("Destroy WavBufInfo");
		}

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


		bool StartPlayBuf(std::shared_ptr<WavBufInfo> spInfo);
		std::shared_ptr<WavBufInfo> StopPlayBuf();


		void SetTcpReceiveSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			m_spSessionReceive = sp;
		}

		bool HasTcpReceiveSession()
		{
			if (m_spSessionReceive != nullptr)
			{
				return true;
			}
			return false;
		}

		bool IsTcpReceiveSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			if (sp == m_spSessionReceive)
			{
				return true;
			}
			return false;
		}

		void AddTcpSendToSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			if (m_spSessionSendToSet.find(sp) == m_spSessionSendToSet.end())
			{
				m_spSessionSendToSet.insert(sp);

			}
		}
		void RemoveTcpSendToSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			if (m_spSessionSendToSet.find(sp) != m_spSessionSendToSet.end())
			{
				m_spSessionSendToSet.erase(sp);

			}
		}
		void BindOnFinishPlayWave(std::function<void(std::shared_ptr<WavBufInfo>)> func)
		{
			m_OnFinishPlayWavFunc = func;

		}
		bool IsPlayingWave()
		{
			if (m_spCurrentWavBufInfo)
			{
				return true;
			}
			return false;
				
		}

	private:

		mal_context* m_pContext;
		mal_device_config m_Config;
		mal_device* m_pCaptureDevice;
		mal_device* m_pPlaybackDevice;


		mal_device_config m_FileConfig;
		mal_device* m_pPlayFileDevice;
		mal_decoder* m_pFileDecoder;


		mal_uint32 capturedSampleCount = 0;
		mal_int16* pCapturedSamples = NULL;
		mal_uint32 playbackSample = 0;

		std::shared_ptr<TcpSocketContainer> m_spSessionReceive;
		std::set<std::shared_ptr<TcpSocketContainer>> m_spSessionSendToSet;

		std::function<void(std::shared_ptr<WavBufInfo>)> m_OnFinishPlayWavFunc;
		std::shared_ptr<WavBufInfo> m_spCurrentWavBufInfo;


		asio::streambuf m_AudioRecvBuf;

		std::mutex m_AudioRecvMutex;

		std::mutex m_AudioBufPlayMutex;

	};


}

#endif // AudioCodec_h__
