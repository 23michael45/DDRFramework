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


	protected:

		virtual void StartRead();
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) override;
		virtual void HandleRead(const asio::error_code& ec);
		virtual void HandleWrite(const asio::error_code&, size_t);


	private:
		int m_TotalRev;
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


		void Start(int threadNum = 2);
		void Stop();

	protected:

		void ThreadEntry();

		void StartAccept();
		void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);

		virtual void OnSessionDisconnect(TcpSocketContainer& container);
		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher();

		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;
		std::map<std::string, std::shared_ptr<TcpSessionBase>> m_SessionMap;

		asio::detail::thread_group m_WorkerThreads;
	private:
		void WaitUntilPreSessionDestroy(std::string ip, std::shared_ptr<TcpSessionBase> spSession);
	};
}

#endif // TcpServerBase_h__
