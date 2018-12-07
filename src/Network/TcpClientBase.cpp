#include "TcpClientBase.h"
#include "../Utility/DDRMacro.h"
namespace DDRFramework
{
	TcpClientBase::TcpClientBase(asio::io_context& context) :m_Resolver(context),m_IOContext(context),TcpSocketContainer::TcpSocketContainer(context)
	{
	}

	TcpClientBase::~TcpClientBase()
	{
	}

	void TcpClientBase::Start(std::string ip, std::string port)
	{
		tcp::resolver::query query(ip, port);
		m_Resolver.async_resolve(query, bind(&TcpClientBase::ResolveHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		

		std::thread t = std::thread(bind(&TcpClientBase::ThreadEntry, shared_from_this()));
		t.detach();
	}

	void TcpClientBase::ThreadEntry()
	{
		m_IOContext.run();
	}
	void TcpClientBase::ResolveHandler(const asio::error_code& ec, tcp::resolver::iterator i)
	{
		if (!ec)
		{
			asio::async_connect(m_Socket, i, bind(&TcpClientBase::ConnectHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			DebugLog(ec.message().c_str());
		}
	}
	void TcpClientBase::ConnectHandler(const asio::error_code& ec, tcp::resolver::iterator i)
	{
		m_bConnected = true;
		m_IOContext.post(std::bind(&TcpClientBase::StartRead, shared_from_this()));

	}
	void TcpClientBase::Send(asio::streambuf& buf)
	{
		if (m_bConnected)
		{
			m_IOContext.post(std::bind(&TcpClientBase::StartWrite, shared_from_this(), std::ref< asio::streambuf>(buf)));
		}

	}
	void TcpClientBase::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf, asio::transfer_at_least(1), std::bind(&TcpClientBase::HandleRead, shared_from_this(), std::placeholders::_1));

	}
	void TcpClientBase::HandleRead(const asio::error_code& ec)
	{
		if (!ec)
		{
			DebugLog("Receive:%i", m_ReadStreamBuf.size());

			GetSerializer()->Receive(m_ReadStreamBuf);
			if (m_bConnected)
			{
				m_IOContext.post(std::bind(&TcpClientBase::StartRead, shared_from_this()));
			}
		}
		else
		{
			m_bConnected = false;

			DebugLog("\nError on receive: :%s", ec.message().c_str());
		}

	}
	void TcpClientBase::StartWrite(asio::streambuf& buf)
	{

		asio::async_write(m_Socket, buf, std::bind(&TcpClientBase::HandleWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
	void TcpClientBase::HandleWrite(const asio::error_code& ec, size_t size)
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
}