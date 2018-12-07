#include "TcpServerBase.h"
#include "../Utility/DDRMacro.h"
#include "../Network/BaseMessageDispatcher.h"
#include "../Network/MessageSerializer.h"
namespace DDRFramework
{

	TcpSessionBase::TcpSessionBase(asio::io_context& context):m_IOContext(context), TcpSocketContainer::TcpSocketContainer(context)
	{

	}
	TcpSessionBase::~TcpSessionBase()
	{

	}

	void TcpSessionBase::Start()
	{	
		DebugLog("\nConnection Established! %s" , m_Socket.remote_endpoint().address().to_string().c_str());

	}
	void TcpSessionBase::Send(asio::streambuf& buf)
	{
		if (m_bConnected)
		{
			m_IOContext.post(std::bind(&TcpSessionBase::StartWrite, shared_from_this(), std::ref< asio::streambuf>(buf)));
		}

	}
	void TcpSessionBase::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf,asio::transfer_at_least(1),std::bind(&TcpSessionBase::HandleRead, shared_from_this(),std::placeholders::_1));

	}
	void TcpSessionBase::HandleRead(const asio::error_code& ec)
	{
		if (!ec)
		{
			DebugLog("Receive:%i", m_ReadStreamBuf.size());

			GetSerializer()->Receive(m_ReadStreamBuf);

			if (m_bConnected)
			{
				m_IOContext.post(std::bind(&TcpSessionBase::StartRead, shared_from_this()));
			}
		}
		else
		{
			m_bConnected = false;

			DebugLog("\nError on receive: :%s", ec.message().c_str());
		}

	}
	void TcpSessionBase::StartWrite(asio::streambuf& buf)
	{

		asio::async_write(m_Socket, buf, std::bind(&TcpSessionBase::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void TcpSessionBase::HandleWrite(const asio::error_code& ec, size_t size)
	{
		if (!ec)
		{
			DebugLog("\nSend :%i", size);
		}
		else
		{
			DebugLog("\nError on send: %s", ec.message().c_str());
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


	void TcpServerBase::StartAccept()
	{

		std::shared_ptr<TcpSessionBase> spSession = std::make_shared<TcpSessionBase>(m_IOContext);
		std::shared_ptr<MessageSerializer> spSerializer = std::make_shared<MessageSerializer>();
		spSession->SetSerializer(spSerializer);
		auto spDispatcher = std::make_shared<BaseMessageDispatcher>(spSession);
		spSerializer->BindDispatcher(spDispatcher);

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
}