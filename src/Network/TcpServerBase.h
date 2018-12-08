#ifndef TcpServerBase_h__
#define TcpServerBase_h__
#include "asio.hpp"
#include <map>
#include "TcpSocketContainer.h"
using asio::ip::tcp;

namespace DDRFramework
{

	class TcpServerBase;
	class TcpSessionBase : public TcpSocketContainer
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

		auto shared_from_base() {
			return std::static_pointer_cast<TcpSessionBase>(shared_from_this());
		}
	};

	class TcpServerBase : public std::enable_shared_from_this<TcpServerBase>
	{
	public:
		TcpServerBase(int port);
		~TcpServerBase();


		void Start();

	protected:

		void ThreadEntry();

		void StartAccept();
		void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);

		virtual void OnSessionDisconnect(std::string remoteAddress);
		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher();

		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;
		std::map<std::string, std::shared_ptr<TcpSessionBase>> m_SessionMap;
		
	private:
	};
}

#endif // TcpServerBase_h__
