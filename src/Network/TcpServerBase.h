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


		SHARED_FROM_BASE(TcpSessionBase)
	};

	class TcpServerBase : public std::enable_shared_from_this<TcpServerBase>
	{
	public:
		TcpServerBase(int port);
		~TcpServerBase();


		void Start(int threadNum = 2);
		void Stop();


		std::shared_ptr<TcpSessionBase> GetTcpSessionBySocket(tcp::socket* pSocket);
		std::vector<std::shared_ptr<TcpSessionBase>> GetConnectedSessions();
	protected:

		void ThreadEntry();

		std::shared_ptr<TcpSessionBase> StartAccept();
		virtual void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);


		virtual void OnSessionDisconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher();

		std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>>& GetTcpSocketContainerMap();

		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;
		
		std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>> m_SessionMap;
		
		
		asio::detail::thread_group m_WorkerThreads;

	
	private:

		std::mutex m_mapMutex;

	};




	class HookTcpSession : public TcpSessionBase
	{
	public:
		HookTcpSession(asio::io_context& context);
		~HookTcpSession();

		SHARED_FROM_BASE(HookTcpSession)

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



		SHARED_FROM_BASE(HookTcpServer)


	protected:

	};
}

#endif // TcpServerBase_h__
