#ifndef UdpSocketBase_h__
#define UdpSocketBase_h__


#include <google/protobuf/message.h>
#include "asio.hpp"
#include <map>
#include <memory>
#include "MessageSerializer.h"
#include "BaseMessageDispatcher.h"

namespace DDRFramework
{
	class UdpSocketBase :public std::enable_shared_from_this<UdpSocketBase>
	{
	public:
		UdpSocketBase();
		~UdpSocketBase();




		void Start();
		void Stop();
		void DelayStop();
		void ThreadEntry();
		bool IsWorking();


		void StartBroadcast(int port,std::shared_ptr<google::protobuf::Message> spMsg, int intervalintervalMillisecond);
		void StopBroadcast();


		void StartReceive(int port);
		void StopReceive();
		void BindOnDisconnect(std::function<void(UdpSocketBase&)> f)
		{
			m_fOnDisconnect = f;
		}

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);

		std::shared_ptr<MessageSerializer> GetSerializer()
		{
			return m_spSerializer;
		}
		asio::io_context& GetIOContext();

		std::shared_ptr<asio::ip::udp::socket> GetSocket()
		{
			return m_spSocket;
		}
	private:

		std::function<void(UdpSocketBase&)> m_fOnDisconnect;

		void StartWrite(std::shared_ptr<asio::streambuf> spbuf);
		void HandleWrite(const asio::error_code& ec, std::shared_ptr<asio::streambuf> spbuf);
		void FreeWrite();
		
		
		void StartRead();
		void HandleRead(const asio::error_code& ec, int len);
		void FreeRead();


		asio::io_context m_IOContext;
		std::shared_ptr<asio::ip::udp::socket> m_spSocket;

		std::shared_ptr< asio::io_service::work > m_spWork;
		int m_IntervalintervalMillisecond;

		std::shared_ptr<MessageSerializer> m_spSerializer;

		std::shared_ptr<google::protobuf::Message> m_spMsg;
		std::shared_ptr<asio::ip::udp::endpoint> m_spRecvEnderEndpoint;
		std::shared_ptr<asio::ip::udp::endpoint> m_spBroadcastEnderEndpoint;
		bool m_Broadcasting;
		bool m_Receiving;

		//asio::streambuf m_ReadStreamBuf; 
		asio::detail::array<char,1024> m_ReadStreamBuf;


		std::mutex m_UdpMutex;
		
		asio::io_context::strand m_ReadWriteStrand;
	};

}

#endif // UdpSocketBase_h__