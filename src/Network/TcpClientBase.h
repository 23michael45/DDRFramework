#ifndef TcpClientBase_h__
#define TcpClientBase_h__
#include "asio.hpp"
#include "MessageSerializer.h"
#include "TcpSocketContainer.h"
#include <memory>

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
		void StartRead();
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) override;
		void HandleRead(const asio::error_code& ec);
		void HandleWrite(const asio::error_code&, size_t);


	private:
		tcp::resolver m_Resolver;
		asio::streambuf m_ReadStreamBuf;
		int m_TotalSend; int m_TotalSendWill;

		auto shared_from_base() {
			return std::static_pointer_cast<TcpClientSessionBase>(shared_from_this());
		}
	};



	class TcpClientBase : public std::enable_shared_from_this<TcpClientBase>
	{
	public:
		TcpClientBase();
		~TcpClientBase();
		void Start(int threadNum = 2);
		void Connect(std::string address, std::string port);
		void Disconnect();
		void Stop();
		void ThreadEntry();

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);

		bool IsConnected()
		{
			if (m_spClient && m_spClient->IsConnected())
			{
				return true;
			}
			return false;
		}
	protected:

		virtual void OnDisconnect(TcpSocketContainer& container);
		virtual void OnConnected(TcpSocketContainer& container);
		virtual std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher();
	
		asio::io_context m_IOContext;
		std::shared_ptr<TcpClientSessionBase> m_spClient;
		std::shared_ptr< asio::io_service::work > m_spWork;
		std::string m_Address;
		std::string m_Port;


		asio::detail::thread_group m_WorkerThreads;
	};
}
#endif // TcpClientBase_h__
