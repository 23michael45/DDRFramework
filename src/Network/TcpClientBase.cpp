#include "TcpClientBase.h"
#include "../Utility/DDRMacro.h"
namespace DDRFramework
{
	TcpClientSessionBase::TcpClientSessionBase(asio::io_context& context) :m_Resolver(context),TcpSocketContainer::TcpSocketContainer(context)
	{
		m_TotalSend = 0; m_TotalSendWill = 0;
	}

	TcpClientSessionBase::~TcpClientSessionBase()
	{
		DebugLog("\nTcpClientBase Destroy");
	}

	void TcpClientSessionBase::Start(std::string ip, std::string port)
	{

		tcp::resolver::query query(ip, port);
		m_Resolver.async_resolve(query, bind(&TcpClientSessionBase::ResolveHandler, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
		
	}

	void TcpClientSessionBase::ResolveHandler(const asio::error_code& ec, tcp::resolver::iterator i)
	{
		if (!ec)
		{
			asio::async_connect(m_Socket, i, bind(&TcpClientSessionBase::ConnectHandler, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
		}
		else
		{
			DebugLog(ec.message().c_str());
		}
	}
	void TcpClientSessionBase::ConnectHandler(const asio::error_code& ec, tcp::resolver::iterator i)
	{
		if (!ec)
		{
			m_bConnected = true;
			TcpSocketContainer::Start();
			//m_IOContext.post(std::bind(&TcpClientSessionBase::StartRead, shared_from_base()));
		}
		else
		{
			DebugLog("\nConnect Failed No Server");
			m_bConnected = false;
			if (m_fOnSessionDisconnect)
			{
				m_fOnSessionDisconnect("");
			}
		}

	}

	void TcpClientSessionBase::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf, asio::transfer_at_least(1), std::bind(&TcpClientSessionBase::HandleRead, shared_from_base(), std::placeholders::_1));

	}
	void TcpClientSessionBase::HandleRead(const asio::error_code& ec)
	{
		try
		{
			if (!ec)
			{
				//DebugLog("\nReceive:%i", m_ReadStreamBuf.size());

				GetSerializer()->Receive(m_ReadStreamBuf);
				if (m_bConnected)
				{
					m_IOContext.post(std::bind(&TcpClientSessionBase::StartRead, shared_from_base()));
				}
			}
			else
			{
				DebugLog("\nError on receive: :%s", ec.message().c_str());
				m_bConnected = false;
				if (m_fOnSessionDisconnect)
				{
					m_fOnSessionDisconnect(m_Socket.remote_endpoint().address().to_string().c_str());
				}

			}
		}
		catch (std::exception* e)
		{
			DebugLog("\nError  :%s", e->what());
			
		}
		catch (asio::system_error* e)
		{
			DebugLog("\nError  :%s", e->what());

		}

	}
	void TcpClientSessionBase::StartWrite(asio::streambuf& buf)
	{
		std::lock_guard<std::mutex> lock(GetSerializer()->GetSendLock());

		m_TotalSendWill += buf.size();
		DebugLog("\nSendWill:%i TotalSendWill:%i", buf.size(), m_TotalSendWill);
		asio::async_write(m_Socket, buf, std::bind(&TcpClientSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
	}
	void TcpClientSessionBase::HandleWrite(const asio::error_code& ec, size_t size)
	{
		if (!ec)
		{

			m_TotalSend += size;
			DebugLog("\nSend:%i TotalSend:%i", size, m_TotalSend);
		
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
	void TcpClientSessionBase::OnDisconnect(std::string remoteAddress)
	{
		UnloadSerializer();
	}




	TcpClientBase::TcpClientBase()
	{

	}
	TcpClientBase::~TcpClientBase()
	{
		DebugLog("\n TcpClientBase Destroy");
		m_spClient.reset();
	}
	void TcpClientBase::Start(std::string address, std::string port)
	{
		m_Address = address;
		m_Port = port;
		std::thread t = std::thread(bind(&TcpClientBase::ThreadEntry, shared_from_this()));
		t.detach();
	}
	void TcpClientBase::Stop()
	{
		m_spClient.reset();
		m_IOContext.stop();
	}
	void TcpClientBase::ThreadEntry()
	{
		try
		{
			m_IOContext.post(std::bind(&TcpClientBase::Update, shared_from_this()));
			m_IOContext.run();
		}
		catch (asio::system_error* e)
		{
			DebugLog("\nError: %s", e->what());

		}
		DebugLog("\nThreadEntry Finish");
	}
	std::shared_ptr<TcpClientSessionBase> TcpClientBase::BindSerializerDispatcher()
	{
		BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, TcpClientSessionBase, MessageSerializer, BaseMessageDispatcher)
		return spTcpClientSessionBase;
	}
	void TcpClientBase::Update()
	{
		if (m_spClient)
		{
		}
		else
		{
			auto spTcpClientBase = BindSerializerDispatcher();

				m_spClient = spTcpClientBase;
			spTcpClientBase->BindOnDisconnect(std::bind(&TcpClientBase::OnDisconnect, shared_from_this(), std::placeholders::_1));
			spTcpClientBase->Start(m_Address, m_Port);
		}

		if (IsConnected())
		{
			m_IOContext.post(std::bind(&TcpClientBase::Update, shared_from_this()));

		}
	}


	void TcpClientBase::Send(google::protobuf::Message& msg)
	{
		if (m_spClient)
		{
			m_spClient->Send(msg);

		}
	}
	void TcpClientBase::OnDisconnect(std::string addr)
	{
		m_spClient->UnloadSerializer();
		m_spClient.reset();
		DebugLog("\nUse Count%i", m_spClient.use_count());
	}

}