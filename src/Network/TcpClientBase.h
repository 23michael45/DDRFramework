/*!
 * File: TcpClientBase.h
 * Date: 2019/04/29 15:53
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Tcp Client Base Class ,Tcp Client Session Base Class
 *
*/
#ifndef TcpClientBase_h__
#define TcpClientBase_h__
#include <memory>
#include "thirdparty/asio/include/asio.hpp"
#include "src/Network/MessageSerializer.h"
#include "src/Network/TcpSocketContainer.h"
#include "src/Utility/LoggerDef.h"
#include "src/Utility/Timer.hpp"
using asio::ip::tcp;
namespace DDRFramework
{


	class TcpClientSessionBase : public TcpSocketContainer
	{
	public:
		TcpClientSessionBase(asio::io_context& context);
		~TcpClientSessionBase();

		virtual void Start(std::string ip, std::string port);
	protected:
		
		void ResolveHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		void ConnectHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		virtual void StartRead();
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) override;
		virtual void HandleRead(const asio::error_code& ec);

		void ConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer);

		asio::streambuf& GetRecvBuf()
		{
			return m_ReadStreamBuf;
		}
	protected:

		tcp::resolver m_Resolver;
		asio::streambuf m_ReadStreamBuf;

		DDRFramework::Timer m_Timer;
		DDRFramework::timer_id m_ConnectTimeoutTimerID;

		SHARED_FROM_BASE(TcpClientSessionBase)
	};
	class HookTcpClientSession : public TcpClientSessionBase
	{
	public:
		HookTcpClientSession(asio::io_context& context);
		~HookTcpClientSession();

		SHARED_FROM_BASE(HookTcpClientSession)

		virtual void OnHookReceive(asio::streambuf& buf) {};

		virtual void StartRead() override;
		virtual void HandleRead(const asio::error_code& ec) override;
	protected:

	};



	class TcpClientBase : public std::enable_shared_from_this<TcpClientBase>
	{
	public:
		TcpClientBase();
		~TcpClientBase();
		void Start(int threadNum = 2);
		std::shared_ptr<TcpClientSessionBase> Connect(std::string address, std::string port);
		void Disconnect(std::shared_ptr<TcpSocketContainer> spContainer = nullptr);
		void Stop();
		void ThreadEntry();


		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<asio::streambuf> spbuf);
		void Send(const void* psrc, int len);

		bool IsConnected()
		{
			std::lock_guard<std::mutex> lock(m_MapMutex);
			for (auto spSession : m_spClientSet)
			{
				if (spSession->IsConnected())
				{
					return true;
				}

			}
			return false;
		}
		std::shared_ptr<TcpClientSessionBase> GetConnectedSession()
		{
			std::lock_guard<std::mutex> lock(m_MapMutex);
			for (auto spSession : m_spClientSet)
			{
				if (spSession->IsConnected())
				{
					return dynamic_pointer_cast<TcpClientSessionBase>(spSession);
				}

			}
			return nullptr;
		}
	protected:

		virtual void OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual void OnConnected(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual void OnConnectTimeout(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual void OnConnectFailed(std::shared_ptr<TcpSocketContainer> spContainer);

		virtual std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher();
	
		asio::io_context m_IOContext;
		std::set< std::shared_ptr<TcpSocketContainer>> m_spClientSet;
		std::mutex m_MapMutex;


		std::shared_ptr< asio::io_service::work > m_spWork;
		std::string m_Address;
		std::string m_Port;


		asio::detail::thread_group m_WorkerThreads;
	};
}
#endif // TcpClientBase_h__
