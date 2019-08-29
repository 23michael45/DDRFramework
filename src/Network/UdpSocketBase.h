/*!
 * File: UdpSocketBase.h
 * Date: 2019/04/29 15:52
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:UdpSocket Base Class
 *
*/
#ifndef UdpSocketBase_h__
#define UdpSocketBase_h__


#include "thirdparty/asio/include/asio.hpp"
#include <map>
#include <memory>
#include "src/Network/MessageSerializer.h"
#include "src/Network/BaseMessageDispatcher.h"
#include "src/Utility/LoggerDef.h"

namespace DDRFramework
{
	class UdpSocketBase :public std::enable_shared_from_this<UdpSocketBase>
	{
	public:
		UdpSocketBase();
		~UdpSocketBase();




		virtual void Start();
		virtual void Stop();
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

		std::shared_ptr<asio::ip::udp::socket> GetSendSocket()
		{
			return m_spSendSocket;
		}
		std::shared_ptr<asio::ip::udp::socket> GetRecvSocket()
		{
			return m_spRecvSocket;
		}
		void BindOnHookReceive(std::function<void(asio::detail::array<char, 1024>&, int)> f)
		{
			m_fOnHookReceiveData = f;
		}
	private:

		std::function<void(UdpSocketBase&)> m_fOnDisconnect;

		void StartWrite(std::shared_ptr<asio::streambuf> spbuf);
		void HandleWrite(const asio::error_code& ec, std::shared_ptr<asio::streambuf> spbuf);
		void FreeWrite();
		
		
		void StartRead();
		void DelayStart(int port);
		void HandleRead(const asio::error_code& ec, int len);
		void FreeRead();


		asio::io_context m_IOContext;
		std::shared_ptr<asio::ip::udp::socket> m_spSendSocket;
		std::shared_ptr<asio::ip::udp::socket> m_spRecvSocket;

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

		std::function<void(asio::detail::array<char, 1024>&,int)> m_fOnHookReceiveData;
	};

}

#endif // UdpSocketBase_h__