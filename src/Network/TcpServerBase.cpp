#include "TcpServerBase.h"
#include "../Utility/DDRMacro.h"
#include "../Utility/Logger.h"
#include "../Network/BaseMessageDispatcher.h"
#include "../Network/MessageSerializer.h"
namespace DDRFramework
{

	TcpSessionBase::TcpSessionBase(asio::io_context& context):TcpSocketContainer::TcpSocketContainer(context)
	{
		m_bConnected = true;
	}
	TcpSessionBase::~TcpSessionBase()
	{

		DebugLog("TcpSessionBase Destroy");
	}

	void TcpSessionBase::Start()
	{	
		DebugLog("Connection Established! %s" , m_Socket.remote_endpoint().address().to_string().c_str());
		TcpSocketContainer::Start();

		if (m_bConnected)
		{

			if (m_fOnSessionConnected)
			{
				m_fOnSessionConnected(shared_from_this());
			}

			m_IOContext.post(std::bind(&TcpSessionBase::StartRead, shared_from_base()));
		}
	}

	void TcpSessionBase::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf,asio::transfer_at_least(1),std::bind(&TcpSessionBase::HandleRead, shared_from_base(),std::placeholders::_1));

	}
	void TcpSessionBase::HandleRead(const asio::error_code& ec)
	{
		if (!ec)
		{
			if (m_bConnected)
			{
				PushData(m_ReadStreamBuf);
				m_ReadWriteStrand.post(std::bind(&TcpSessionBase::StartRead, shared_from_base()));
			}
		}
		else
		{
			Stop();

			DebugLog("TcpSessionBase Error on receive: %s", ec.message().c_str());
		}

	}
	void TcpSessionBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{

		//DebugLog("StartWrite %d", spbuf->size());
		if (m_Socket.is_open())
		{

			asio::async_write(m_Socket, *spbuf.get(), std::bind(&TcpSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));

		}

	}

	TcpServerBase::TcpServerBase(int port) :m_Acceptor(m_IOContext, tcp::endpoint(tcp::v4(), port))
	{
		DebugLog("TcpServerBase  Create");
	}

	TcpServerBase::~TcpServerBase()
	{
		DebugLog("TcpServerBase  Destroy");
	}


	void TcpServerBase::Start(int threadNum)
	{
		m_WorkerThreads.create_threads(std::bind(&TcpServerBase::ThreadEntry, shared_from_this()), threadNum);
	}
	void TcpServerBase::Stop()
	{
		std::vector<std::shared_ptr<TcpSessionBase>> vec;

		for (auto spSessionPair : m_SessionMap)
		{
			auto spSession = spSessionPair.second;
			vec.push_back(spSession);
		}//donot clear map heae , session stop will call ondisconncet callback ,and session ptr will be remove in ondisconnect function

		for (auto spSession : vec)
		{

			spSession->Stop();
		}
	}

	void TcpServerBase::ThreadEntry()
	{
		StartAccept();
		m_IOContext.run(); 
	}


	std::shared_ptr<TcpSessionBase> TcpServerBase::BindSerializerDispatcher()
	{
		BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, TcpSessionBase, MessageSerializer, BaseMessageDispatcher,BaseHeadRuleRouter)
			return spTcpSessionBase;
	}
	std::shared_ptr<TcpSessionBase> TcpServerBase::StartAccept()
	{
		auto spSession = BindSerializerDispatcher();
		spSession->BindOnDisconnect(std::bind(&TcpServerBase::OnSessionDisconnect, shared_from_this(), std::placeholders::_1));
		m_Acceptor.async_accept(spSession->GetSocket(),
			std::bind(&TcpServerBase::HandleAccept, this, spSession,std::placeholders::_1));

		return spSession;

	}
	void TcpServerBase::HandleAccept(std::shared_ptr<TcpSessionBase> spSession, const asio::error_code& error)
	{
		if (!error)
		{ 
			if (m_SessionMap.find(&spSession->GetSocket()) == m_SessionMap.end())
			{
				m_SessionMap[&spSession->GetSocket()] = spSession;
				spSession->Start();

			}
			else
			{
				/*m_SessionMap[spSession->GetSocket()]->Stop();
				std::thread t(bind(&TcpServerBase::WaitUntilPreSessionDestroy, shared_from_this(), ip, spSession));
				t.detach();*/


			}
		}

		StartAccept();
	}
	//void TcpServerBase::WaitUntilPreSessionDestroy(tcp::socket& socket, std::shared_ptr<TcpSessionBase> spSession)
	//{
	//	while (m_SessionMap.find(spSession->GetSocket()) != m_SessionMap.end())
	//	{
	//		std::this_thread::sleep_for(std::chrono::seconds(1));
	//	}

	//	m_SessionMap[socket] = spSession;
	//	spSession->Start();
	//}


	void TcpServerBase::OnSessionDisconnect(std::shared_ptr<TcpSocketContainer> spContainer)
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		try
		{
			asio::ip::tcp::socket& sock = spContainer->GetSocket();
			asio::ip::tcp::socket* psock = &sock;
			if (m_SessionMap.find(psock) != m_SessionMap.end())
			{
				auto sp = m_SessionMap[psock];
				//erase first then release and reset  , cause somewhere may call map to get session,it will get a empty sp if reset first
				m_SessionMap.erase(psock);
				sp->Release();
				sp.reset();
			}
		}
		catch (asio::error_code& e)
		{
			DebugLog("Disconnect Error %s", e.message().c_str())
		}
		catch (asio::system_error& e)
		{
			DebugLog("Disconnect Error %s", e.what())
		}
		catch (std::exception& e)
		{

			DebugLog("Disconnect Error %s", e.what())
		}
	}

	std::shared_ptr<TcpSessionBase> TcpServerBase::GetTcpSessionBySocket(tcp::socket* pSocket)
	{
		if (m_SessionMap.find(pSocket) != m_SessionMap.end())
		{
			return m_SessionMap[pSocket];
		}
		return nullptr;
	}

	std::vector<std::shared_ptr<DDRFramework::TcpSessionBase>> TcpServerBase::GetConnectedSessions()
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		std::vector<std::shared_ptr<TcpSessionBase>> vec;
		for (auto spPair : m_SessionMap)
		{
			vec.push_back(spPair.second);
		}
		return vec;
	}

	std::map<tcp::socket*, std::shared_ptr<TcpSessionBase>>& TcpServerBase::GetTcpSocketContainerMap()
	{

		return m_SessionMap;
	}












	//-------------------------------------------------------------------------------------------------------------------------------------------------






	HookTcpSession::HookTcpSession(asio::io_context& context) : DDRFramework::TcpSessionBase(context)
	{
		SetRealtime(true);
	}
	HookTcpSession::~HookTcpSession()
	{
		DebugLog("HookTcpSession Destroy")
	}
	void HookTcpSession::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf, asio::transfer_at_least(1), std::bind(&HookTcpSession::HandleRead, shared_from_base(), std::placeholders::_1));

	}
	void HookTcpSession::HandleRead(const asio::error_code& ec)
	{
		if (!ec)
		{
			if (m_bConnected)
			{
				OnHookReceive(m_ReadStreamBuf);
				m_ReadStreamBuf.consume(m_ReadStreamBuf.size());
				m_ReadWriteStrand.post(std::bind(&HookTcpSession::StartRead, shared_from_base()));
			}
		}
		else
		{
			Stop();
			DebugLog("HookTcpSession Error on receive:  %s",  ec.message().c_str());
		}
	}




	HookTcpServer::HookTcpServer(int port) :TcpServerBase(port)
	{
	}


	HookTcpServer::~HookTcpServer()
	{
		DebugLog("HookTcpServer Destroy")
	}


	std::shared_ptr<TcpSessionBase> HookTcpServer::BindSerializerDispatcher()
	{
		BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, HookTcpSession, MessageSerializer, BaseMessageDispatcher, BaseHeadRuleRouter)
			return spHookTcpSession;
	}

}