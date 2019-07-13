/*!
 * File: TcpServerBase.h
 * Date: 2019/04/29 15:53
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:TcpServer Base Class,Tcp Session Class
 *
*/
#ifndef TcpServerBase_h__
#define TcpServerBase_h__
#include <map>
#include "asio.hpp"
#include "src/Utility/LoggerDef.h"
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
		virtual void OnStart();


		asio::streambuf& GetRecvBuf()
		{
			return m_ReadStreamBuf;
		}
		void SetReadBufSize(size_t size)
		{
			m_ReadStreamBuf.prepare(size);
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


		virtual void Start(int threadNum = 2);
		virtual void Stop();


		std::shared_ptr<TcpSessionBase> GetTcpSessionBySocket(tcp::socket* pSocket);
		std::vector<std::shared_ptr<TcpSessionBase>> GetConnectedSessions();
		int SendToAll(std::shared_ptr<asio::streambuf> spbuf);
	protected:

		void ThreadEntry();

		std::shared_ptr<TcpSessionBase> StartAccept();
		virtual void HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error);


		virtual void OnSessionDisconnect(std::shared_ptr<TcpSocketContainer> spContainer);
		virtual std::shared_ptr<TcpSessionBase> BindSerializerDispatcher();

		std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>>& GetTcpSocketContainerMap();

		asio::io_context m_IOContext;
		tcp::acceptor m_Acceptor;
		

		std::set< std::shared_ptr<TcpSessionBase>> m_WaitingAcceptSessionSet;
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
