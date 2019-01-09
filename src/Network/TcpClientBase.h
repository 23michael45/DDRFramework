#ifndef TcpClientBase_h__
#define TcpClientBase_h__
#include "asio.hpp"
#include "MessageSerializer.h"
#include "TcpSocketContainer.h"
#include <memory>
#include "../../Shared/proto/BaseCmd.pb.h"

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


		asio::streambuf& GetRecvBuf()
		{
			return m_ReadStreamBuf;
		}
	protected:

		tcp::resolver m_Resolver;
		asio::streambuf m_ReadStreamBuf;

		auto shared_from_base() {
			return std::static_pointer_cast<TcpClientSessionBase>(shared_from_this());
		}
	};
	class HookTcpClientSession : public TcpClientSessionBase
	{
	public:
		HookTcpClientSession(asio::io_context& context);
		~HookTcpClientSession();

		auto shared_from_base() {
			return std::static_pointer_cast<HookTcpClientSession>(shared_from_this());
		}

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
		void Disconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		void Stop();
		void ThreadEntry();


		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<asio::streambuf> spbuf);
		void Send(const void* psrc, int len);

		bool IsConnected()
		{
			std::lock_guard<std::mutex> lock(m_MapMutex);
			for (auto spSession : m_spClientMap)
			{
				if (spSession.second->IsConnected())
				{
					return true;
				}

			}
			return false;
		}
		std::shared_ptr<TcpClientSessionBase> GetConnectedSession()
		{
			std::lock_guard<std::mutex> lock(m_MapMutex);
			for (auto spSession : m_spClientMap)
			{
				if (spSession.second->IsConnected())
				{
					return spSession.second;
				}

			}
			return nullptr;
		}
	protected:

		virtual void OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual void OnConnected(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher();
	
		asio::io_context m_IOContext;
		std::map< std::shared_ptr<TcpSocketContainer>, std::shared_ptr<TcpClientSessionBase>> m_spClientMap;
		std::mutex m_MapMutex;


		std::shared_ptr< asio::io_service::work > m_spWork;
		std::string m_Address;
		std::string m_Port;


		asio::detail::thread_group m_WorkerThreads;
	};
}
#endif // TcpClientBase_h__
