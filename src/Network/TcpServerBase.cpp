#include "TcpServerBase.h"
#include "../Utility/DDRMacro.h"
#include "../Network/BaseMessageDispatcher.h"
#include "../Network/MessageSerializer.h"
namespace DDRFramework
{

	TcpSessionBase::TcpSessionBase(asio::io_context& context):TcpSocketContainer::TcpSocketContainer(context)
	{
		m_TotalRev = 0;
		m_bConnected = true;
	}
	TcpSessionBase::~TcpSessionBase()
	{

		DebugLog("\nTcpSessionBase Destroy");
	}

	void TcpSessionBase::Start()
	{	
		DebugLog("\nConnection Established! %s" , m_Socket.remote_endpoint().address().to_string().c_str());
		TcpSocketContainer::Start();

		if (m_bConnected)
		{

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
			m_TotalRev += m_ReadStreamBuf.size();
			DebugLog("\nReceive:%i TotalRev:%i", m_ReadStreamBuf.size(), m_TotalRev);

			PushData(m_ReadStreamBuf);

			if (m_bConnected)
			{
				m_ReadWriteStrand.post(std::bind(&TcpSessionBase::StartRead, shared_from_base()));
			}
		}
		else
		{
			Stop();

			DebugLog("\nError on receive: :%s", ec.message().c_str());
		}

	}
	void TcpSessionBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{

		if (m_Socket.is_open())
		{

			asio::async_write(m_Socket, *spbuf.get(), std::bind(&TcpSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));

		}

	}
	void TcpSessionBase::HandleWrite(const asio::error_code& ec, size_t size)
	{
		std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());
		if (!ec)
		{
			m_spSerializer->PopSendBuf();
			//DebugLog("\nSend :%i", size);
		}
		else
		{
			DebugLog("\nError on send: %s", ec.message().c_str());

			Stop();
			
		}

		if (m_bConnected)
		{
			m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));

		}
	}
	 

	TcpServerBase::TcpServerBase(int port) :m_Acceptor(m_IOContext, tcp::endpoint(tcp::v4(), port))
	{
	}

	TcpServerBase::~TcpServerBase()
	{
	}


	void TcpServerBase::Start(int threadNum)
	{
		m_WorkerThreads.create_threads(std::bind(&TcpServerBase::ThreadEntry, shared_from_this()), threadNum);
	}
	void TcpServerBase::Stop()
	{

		std::vector<tcp::socket*> keyvec;

		for (auto spSessionPair : m_SessionMap)
		{
			spSessionPair.second->Stop();
			keyvec.push_back(spSessionPair.first);
		}

		for (auto iter = keyvec.begin(); iter != keyvec.end(); ++iter)
		{
			m_SessionMap.erase(*iter);
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
	void TcpServerBase::StartAccept()
	{
		auto spSession = BindSerializerDispatcher();
		spSession->BindOnDisconnect(std::bind(&TcpServerBase::OnSessionDisconnect, shared_from_this(), std::placeholders::_1));
		m_Acceptor.async_accept(spSession->GetSocket(),
			std::bind(&TcpServerBase::HandleAccept, this, spSession,std::placeholders::_1));

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


	void TcpServerBase::OnSessionDisconnect(TcpSocketContainer& container)
	{
		try
		{
			if (m_SessionMap.find(&container.GetSocket()) != m_SessionMap.end())
			{
				auto sp = m_SessionMap[&container.GetSocket()];
				sp->Release();
				sp.reset();
				m_SessionMap.erase(&container.GetSocket());
			}
		}
		catch (asio::error_code& e)
		{
			DebugLog("\nDisconnect Error %s", e.message().c_str())
		}
		catch (asio::system_error& e)
		{
			DebugLog("\nDisconnect Error %s", e.what())
		}
		catch (std::exception& e)
		{

			DebugLog("\nDisconnect Error %s", e.what())
		}
	}
}