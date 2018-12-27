#ifndef TcpServerBase_h__
#define TcpServerBase_h__
#include "asio.hpp"
#include <map>
#include "../../proto/BaseCmd.pb.h"
#include "TcpSocketContainer.h"
using asio::ip::tcp;
using namespace DDRCommProto;

namespace DDRFramework
{

	class TcpServerBase;
	class TcpSessionBase : public TcpSocketContainer
	{
	public:
		TcpSessionBase(asio::io_context& context);
		~TcpSessionBase();
		virtual void Start();

		void AssignLoginInfo(reqLogin info)
		{
			m_reqLoginInfo.CopyFrom(info);
		}
		reqLogin& GetLoginInfo()
		{
			return m_reqLoginInfo;
		}

		asio::streambuf& GetRecvBuf()
		{
			return m_ReadStreamBuf;
		}
	protected:

		virtual void StartRead();
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) override;
		virtual void HandleRead(const asio::error_code& ec);


		

	protected:
		asio::streambuf m_ReadStreamBuf;
		reqLogin m_reqLoginInfo;//Login Information


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


		std::shared_ptr<TcpSessionBase> GetTcpSessionBySocket(tcp::socket* pSocket);
		std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>>& GetTcpSocketContainerMap();
	protected:

		void ThreadEntry();

		virtual std::shared_ptr<TcpSessionBase> StartAccept();
		void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);


		virtual void OnSessionDisconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher();

		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;
		
		std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>> m_SessionMap;
		
		
		asio::detail::thread_group m_WorkerThreads;

	
	private:
		//void WaitUntilPreSessionDestroy(tcp::socket& socket, std::shared_ptr<TcpSessionBase> spSession);
	};




	class HookTcpSession : public TcpSessionBase
	{
	public:
		HookTcpSession(asio::io_context& context);
		~HookTcpSession();

		auto shared_from_base() {
			return std::static_pointer_cast<HookTcpSession>(shared_from_this());
		}

		virtual void OnHookReceive(asio::streambuf& buf) {};

		virtual void StartRead() override;
		virtual void HandleRead(const asio::error_code& ec) override;
	protected:
	};


	class HookTcpServer : public TcpServerBase
	{
	public:
		HookTcpServer(int port);
		~HookTcpServer();


		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher() override;



		auto shared_from_base() {
			return std::static_pointer_cast<HookTcpServer>(shared_from_this());
		}


		virtual std::shared_ptr<TcpSessionBase> StartAccept() override;

	protected:

	};
}

#endif // TcpServerBase_h__
