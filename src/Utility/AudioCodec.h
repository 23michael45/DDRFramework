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
	class AudioCodec
	{
	public:
		AudioCodec();
		~AudioCodec();

		bool Init();
		void Deinit();

		bool StartRecord();
		void StopRecord();
		bool StartPlay();
		void StopPlay();



		void on_recv_frames_device(mal_device* pDevice, mal_uint32 frameCount, const void* pSamples);
		mal_uint32 on_send_frames_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);


		mal_uint32 on_send_frames_to_device(mal_device* pDevice, mal_uint32 frameCount, void* pSamples);


		void StartPlayFile(asio::streambuf& buf);
		void StartPlayFile(std::string fileName);
		void StopPlayFile();



		void PushAudioRecvBuf(asio::streambuf& buf)
		{
			std::lock_guard<std::mutex> lock(m_AudioRecvMutex);
			std::ostream oshold(&m_AudioRecvBuf);
			oshold.write((const char*)buf.data().data(), buf.size());
			oshold.flush();
		}


		void SetTcpSession(std::shared_ptr<TcpSocketContainer> sp)
		{
			m_spSession = sp;
		}

		void PlayAudio(std::shared_ptr<asio::streambuf>, int priority);
		void EndPlay(std::pair<int, std::shared_ptr<asio::streambuf>> pair);
		std::pair<int, std::shared_ptr<asio::streambuf>> GetQueueAudio();
	private:

		struct greater
		{
			bool operator()(const int& _Left, const int& _Right) const
			{
				return (_Left > _Right);
			}
		};

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



		std::map<int, std::shared_ptr<std::queue<std::shared_ptr<asio::streambuf>>>, greater> m_AudioQueueMap;

		std::stack<std::pair<int, std::shared_ptr<asio::streambuf>>> m_CurrentPlayPauseBufStack;

		std::mutex m_AudioRecvMutex;


		std::mutex m_AudioQueueMutex;
		asio::streambuf m_AudioRecvBuf;

	};


}

#endif // AudioCodec_h__
