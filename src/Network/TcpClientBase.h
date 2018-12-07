#ifndef TcpClientBase_h__
#define TcpClientBase_h__
#include "asio.hpp"
#include "MessageSerializer.h"
#include "TcpSocketContainer.h"

using asio::ip::tcp;
namespace DDRFramework
{

	class TcpClientBase : public std::enable_shared_from_this<TcpClientBase>,public TcpSocketContainer
	{
	public:
		TcpClientBase(asio::io_context& context);
		~TcpClientBase();

		virtual void Start(std::string ip, std::string port);
		virtual void Send(asio::streambuf& buf);
	protected:

		virtual void ThreadEntry();

		virtual void ResolveHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		virtual void ConnectHandler(const asio::error_code& ec, tcp::resolver::iterator i);
		virtual void StartRead();
		virtual void StartWrite(asio::streambuf& buf);
		virtual void HandleRead(const asio::error_code& ec);
		virtual void HandleWrite(const asio::error_code&, size_t);

	private:
		asio::io_context& m_IOContext;
		tcp::resolver m_Resolver;
		asio::streambuf m_ReadStreamBuf;
	};
}
#endif // TcpClientBase_h__
