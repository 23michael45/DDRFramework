#ifndef TcpServerBase_h__
#define TcpServerBase_h__
#include "asio.hpp"
#include <map>
#include "TcpSocketContainer.h"
using asio::ip::tcp;

namespace DDRFramework
{

	class TcpServerBase;
	class TcpSessionBase : public std::enable_shared_from_this<TcpSessionBase>, public TcpSocketContainer
	{
	public:
		TcpSessionBase(asio::io_context& context);
		~TcpSessionBase();
		virtual void Start();
		virtual void Send(asio::streambuf& buf);
	

	protected:

		virtual void StartRead();
		virtual void StartWrite(asio::streambuf& buf);
		virtual void HandleRead(const asio::error_code& ec);
		virtual void HandleWrite(const asio::error_code&, size_t);


	private:
		asio::streambuf m_ReadStreamBuf;
		asio::io_context& m_IOContext;
	};

	class TcpServerBase : public std::enable_shared_from_this<TcpServerBase>
	{
	public:
		TcpServerBase(int port);
		~TcpServerBase();


		virtual void Start();

	protected:

		virtual void ThreadEntry();


	private:
		virtual void StartAccept();
		virtual void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);




		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;

		std::map<std::string , std::shared_ptr<TcpSessionBase>> m_SessionMap;
	};
}

#endif // TcpServerBase_h__
