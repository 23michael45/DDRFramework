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
		void OnDisconnect(std::string remoteAddress);
	protected:
		
		void ResolveHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		void ConnectHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		void StartRead();
		void StartWrite(asio::streambuf& buf);
		void HandleRead(const asio::error_code& ec);
		void HandleWrite(const asio::error_code&, size_t);


	private:
		tcp::resolver m_Resolver;
		asio::streambuf m_ReadStreamBuf;

		auto shared_from_base() {
			return std::static_pointer_cast<TcpClientSessionBase>(shared_from_this());
		}
	};



	class TcpClientBase : public std::enable_shared_from_this<TcpClientBase>
	{
	public:
		TcpClientBase();
		~TcpClientBase();
		void Start(std::string address, std::string port);
		void ThreadEntry();
		void Update();

		void Send(google::protobuf::Message& msg);

		bool IsConnected()
		{
			if (m_spClient && m_spClient->IsConnected())
			{
				return true;
			}
			return false;
		}
	protected:

		virtual void OnDisconnect(std::string addr);
		virtual std::shared_ptr<TcpClientSessionBase> BindSerializerDispatcher();
	
		asio::io_context m_IOContext;
		std::shared_ptr<TcpClientSessionBase> m_spClient;

		std::string m_Address;
		std::string m_Port;
	};
}
#endif // TcpClientBase_h__
