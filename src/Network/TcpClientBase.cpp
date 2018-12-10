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
		CloseSocket();
		DebugLog("\nTcpClientSessionBase Destroy");
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
			m_IOContext.post(std::bind(&TcpClientSessionBase::StartRead, shared_from_base()));
		}
		else
		{
			DebugLog("\nConnect Failed No Server");
			m_bConnected = false;
			if (m_fOnSessionDisconnect)
			{
				m_fOnSessionDisconnect(*this);
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

				PushData(m_ReadStreamBuf);
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
					m_fOnSessionDisconnect(*this);
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
	void TcpClientSessionBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{
		DebugLog("\nStart Write");
		if (m_Socket.is_open())
		{
			m_TotalSendWill += spbuf->size();
			asio::async_write(m_Socket, *spbuf.get(),asio::transfer_all(), std::bind(&TcpClientSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
		}

		DebugLog("\nEnd Write");
	}
	void TcpClientSessionBase::HandleWrite(const asio::error_code& ec, size_t size)
	{
		if (m_spSerializer)
		{
			std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());
			if (!ec)
			{
				m_spSerializer->PopSendBuf();
				m_TotalSend += size;
				DebugLog("\nSend:%i TotalSend:%i TotalSendWill:%i", size, m_TotalSend, m_TotalSendWill);

			}
			else
			{
				DebugLog("\nError on send: %s", ec.message().c_str());

				m_bConnected = false;
				/*if (m_fOnSessionDisconnect)
				{
					m_fOnSessionDisconnect(*this);
				}*/
			}

			m_IOContext.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));
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
		DebugLog("\nTcpClientBase Destroy");
		m_spClient.reset();
	}
	void TcpClientBase::Start(int threadNum)
	{
		m_spWork = std::make_shared< asio::io_context::work>(m_IOContext);
		m_WorkerThreads.create_threads(std::bind(&TcpClientBase::ThreadEntry, shared_from_this()), threadNum);
	}
	void TcpClientBase::Connect(std::string address, std::string port)
	{
		m_Address = address;
		m_Port = port;


		auto spTcpClientSessionBase = BindSerializerDispatcher();
		m_spClient = spTcpClientSessionBase;
		
		spTcpClientSessionBase->BindOnDisconnect(std::bind(&TcpClientBase::OnDisconnect, shared_from_this(), std::placeholders::_1));
		spTcpClientSessionBase->Start(m_Address, m_Port);

	}
	void TcpClientBase::Disconnect()
	{
		if (m_spClient && m_spClient->IsConnected())
		{
			m_spClient->CloseSocket();

		}
	}

	void TcpClientBase::Stop()
	{
		m_spWork.reset();
	}
	void TcpClientBase::ThreadEntry()
	{
		try
		{
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

	void TcpClientBase::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_spClient)
		{
			m_spClient->Send(spmsg);

		}
	}
	void TcpClientBase::CheckWrite()
	{
		m_IOContext.post(std::bind(&TcpSocketContainer::CheckWrite, m_spClient));
	}
	void TcpClientBase::OnDisconnect(TcpSocketContainer& container)
	{
		m_spClient->UnloadSerializer();
		m_spClient.reset();
		DebugLog("\nUse Count%i", m_spClient.use_count());
	}

}