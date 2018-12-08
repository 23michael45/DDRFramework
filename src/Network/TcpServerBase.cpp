#include "TcpServerBase.h"
#include "../Utility/DDRMacro.h"
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

		DebugLog("\nTcpSessionBase Destroy %s", m_Socket.remote_endpoint().address().to_string().c_str());
	}

	void TcpSessionBase::Start()
	{	
		DebugLog("\nConnection Established! %s" , m_Socket.remote_endpoint().address().to_string().c_str());
		TcpSocketContainer::Start();

		m_IOContext.post(std::bind(&TcpSessionBase::StartRead, shared_from_base()));
	}
	void TcpSessionBase::Send(asio::streambuf& buf)
	{
		if (m_bConnected)
		{
			m_IOContext.post(std::bind(&TcpSessionBase::StartWrite, shared_from_base(), std::ref< asio::streambuf>(buf)));
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
			//DebugLog("\nReceive:%i", m_ReadStreamBuf.size());

			GetSerializer()->Receive(m_ReadStreamBuf);

			if (m_bConnected)
			{
				m_IOContext.post(std::bind(&TcpSessionBase::StartRead, shared_from_base()));
			}
		}
		else
		{
			m_bConnected = false;
			if (m_fOnSessionDisconnect)
			{
				m_fOnSessionDisconnect(m_Socket.remote_endpoint().address().to_string().c_str());
			}
			DebugLog("\nError on receive: :%s", ec.message().c_str());
		}

	}
	void TcpSessionBase::StartWrite(asio::streambuf& buf)
	{
		std::lock_guard<std::mutex> lock(GetSerializer()->GetSendLock());

		asio::async_write(m_Socket, buf, std::bind(&TcpSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
	}
	void TcpSessionBase::HandleWrite(const asio::error_code& ec, size_t size)
	{
		if (!ec)
		{
			//DebugLog("\nSend :%i", size);
		}
		else
		{
			DebugLog("\nError on send: %s", ec.message().c_str());

			m_bConnected = false;
			if (m_fOnSessionDisconnect)
			{
				m_fOnSessionDisconnect(m_Socket.remote_endpoint().address().to_string().c_str());
			}
		}

	}


	TcpServerBase::TcpServerBase(int port) :m_Acceptor(m_IOContext, tcp::endpoint(tcp::v4(), port))
	{
	}

	TcpServerBase::~TcpServerBase()
	{
	}


	void TcpServerBase::Start()
	{
		std::thread t = std::thread(bind(&TcpServerBase::ThreadEntry,shared_from_this()));
		t.detach();
	}

	void TcpServerBase::ThreadEntry()
	{
		StartAccept();
		m_IOContext.run(); 
	}


	std::shared_ptr<TcpSessionBase> TcpServerBase::BindSerializerDispatcher()
	{
		BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, TcpSessionBase, MessageSerializer, BaseMessageDispatcher)
			return spTcpSessionBase;
	}
	void TcpServerBase::StartAccept()
	{
		auto spSession = BindSerializerDispatcher();
		spSession->BindOnDisconnect(std::bind(&TcpServerBase::OnSessionDisconnect, shared_from_this(), std::placeholders::_1));
		m_Acceptor.async_accept(spSession->GetSocket(),
			std::bind(&TcpServerBase::HandleAccept, this, spSession,std::placeholders::_1));

	}
	void TcpServerBase::HandleAccept(std::shared_ptr<TcpSessionBase> sp, const asio::error_code& error)
	{
		if (!error)
		{
			m_SessionMap[sp->GetSocket().remote_endpoint().address().to_string()] = sp;
			sp->Start();
		}

		StartAccept();
	}
	void TcpServerBase::OnSessionDisconnect(std::string remoteAddress)
	{
		if (m_SessionMap.find(remoteAddress) != m_SessionMap.end())
		{
			auto sp = m_SessionMap[remoteAddress];
			sp->UnloadSerializer();
			DebugLog("\nUse Count: %i", m_SessionMap[remoteAddress].use_count());
			sp.reset();
			m_SessionMap.erase(remoteAddress);
		}
	}
}