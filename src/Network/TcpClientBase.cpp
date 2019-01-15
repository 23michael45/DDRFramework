#include "TcpClientBase.h"
#include "../Utility/DDRMacro.h"
#include "../Utility/Logger.h"
namespace DDRFramework
{
	TcpClientSessionBase::TcpClientSessionBase(asio::io_context& context) :m_Resolver(context),TcpSocketContainer::TcpSocketContainer(context)
	{
	}

	TcpClientSessionBase::~TcpClientSessionBase()
	{
		DebugLog("TcpClientSessionBase Destroy");
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
			m_ReadWriteStrand.post(std::bind(&TcpClientSessionBase::StartRead, shared_from_base()));

			if (m_fOnSessionConnected)
			{
				m_fOnSessionConnected(shared_from_this());
			}
		}
		else
		{
			DebugLog("Connect Failed No Server");
			Stop();
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
			if (m_bConnected)
			{
				if (!ec)
				{
					//DebugLog("Receive:%i", m_ReadStreamBuf.size());

					PushData(m_ReadStreamBuf);
					m_ReadWriteStrand.post(std::bind(&TcpClientSessionBase::StartRead, shared_from_base()));

				}
				else
				{
					DebugLog("TcpClientSessionBase Error on receive: %s : %s", m_Socket.remote_endpoint().address().to_string().c_str(), ec.message().c_str());
					Stop();
				}
			}
		}
		catch (std::exception& e)
		{
			DebugLog("Error  :%s", e.what());
			
		}
		catch (asio::system_error& e)
		{
			DebugLog("Error  :%s", e.what());

		}

	}
	void TcpClientSessionBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{
		if (m_Socket.is_open())
		{
			asio::async_write(m_Socket, *spbuf.get(),asio::transfer_all(), std::bind(&TcpClientSessionBase::HandleWrite, shared_from_base(), std::placeholders::_1, std::placeholders::_2));
		}

	}


	HookTcpClientSession::HookTcpClientSession(asio::io_context& context) : DDRFramework::TcpClientSessionBase(context)
	{
		SetRealtime(true);
	}
	HookTcpClientSession::~HookTcpClientSession()
	{
		DebugLog("HookTcpClientSession Destroy")
	}

	void HookTcpClientSession::StartRead()
	{
		asio::async_read(m_Socket, m_ReadStreamBuf, asio::transfer_at_least(1), std::bind(&HookTcpClientSession::HandleRead, shared_from_base(), std::placeholders::_1));

	}
	void HookTcpClientSession::HandleRead(const asio::error_code& ec)
	{
		try
		{
			if (m_bConnected)
			{
				if (!ec)
				{
					//DebugLog("Receive:%i", m_ReadStreamBuf.size());

					OnHookReceive(m_ReadStreamBuf);

					m_ReadStreamBuf.consume(m_ReadStreamBuf.size());
					m_ReadWriteStrand.post(std::bind(&HookTcpClientSession::StartRead, shared_from_base()));

				}
				else
				{
					DebugLog("TcpClientSessionBase Error on receive: %s : %s", m_Socket.remote_endpoint().address().to_string().c_str(), ec.message().c_str());
					Stop();
				}
			}
		}
		catch (std::exception& e)
		{
			DebugLog("Error  :%s", e.what());

		}
		catch (asio::system_error& e)
		{
			DebugLog("Error  :%s", e.what());

		}
	}



	TcpClientBase::TcpClientBase()
	{
	}
	TcpClientBase::~TcpClientBase()
	{
		std::lock_guard<std::mutex> lock(m_MapMutex);
		DebugLog("TcpClientBase Destroy");
		m_spClientMap.clear();
	}
	void TcpClientBase::Start(int threadNum)
	{
		m_spWork = std::make_shared< asio::io_context::work>(m_IOContext);
		m_WorkerThreads.create_threads(std::bind(&TcpClientBase::ThreadEntry, shared_from_this()), threadNum);
	}
	std::shared_ptr<TcpClientSessionBase> TcpClientBase::Connect(std::string address, std::string port)
	{
		std::lock_guard<std::mutex> lock(m_MapMutex);
		m_Address = address;
		m_Port = port;


		auto spTcpClientSessionBase = BindSerializerDispatcher();

		m_spClientMap.insert(std::make_pair(spTcpClientSessionBase,spTcpClientSessionBase));

		spTcpClientSessionBase->BindOnConnected(std::bind(&TcpClientBase::OnConnected, shared_from_this(), std::placeholders::_1));
		spTcpClientSessionBase->BindOnDisconnect(std::bind(&TcpClientBase::OnDisconnect, shared_from_this(), std::placeholders::_1));
		spTcpClientSessionBase->Start(m_Address, m_Port);

		return spTcpClientSessionBase;
	}
	void TcpClientBase::Disconnect(std::shared_ptr<TcpSocketContainer> spContainer)
	{
		if (IsConnected())
		{
			spContainer->Stop();

		}
	}

	void TcpClientBase::Stop()
	{
		std::lock_guard<std::mutex> lock(m_MapMutex);
		std::vector<std::shared_ptr<TcpClientSessionBase>> vec;
		for (auto spSession : m_spClientMap)
		{
			vec.push_back(spSession.second);
		}
		m_spClientMap.clear();
		for (auto spSession : vec)
		{
			spSession->Stop();
		}
		m_spWork.reset();
	}
	void TcpClientBase::ThreadEntry()
	{
		try
		{
			m_IOContext.run();
		}
		catch (asio::system_error& e)
		{
			DebugLog("Error: %s", e.what());

		}
		DebugLog("ThreadEntry Finish");
	}
	std::shared_ptr<TcpClientSessionBase> TcpClientBase::BindSerializerDispatcher()
	{
		BIND_IOCONTEXT_SERIALIZER_DISPATCHER(m_IOContext, TcpClientSessionBase, MessageSerializer, BaseMessageDispatcher,BaseHeadRuleRouter)

		return spTcpClientSessionBase;
	}

	void TcpClientBase::Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		auto sp = GetConnectedSession();
		if (sp)
		{
			sp->Send(spheader,spmsg);

		}
	}
	void TcpClientBase::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		Send(nullptr, spmsg);
	}
	void TcpClientBase::Send(std::shared_ptr<asio::streambuf> spbuf)
	{
		auto sp = GetConnectedSession();
		if (sp)
		{
			sp->Send(spbuf);

		}
	}
	void TcpClientBase::Send(const void* psrc, int len)
	{
		auto sp = GetConnectedSession();
		if (sp)
		{
			sp->Send(psrc,len);

		}
	}
	void TcpClientBase::OnDisconnect(std::shared_ptr<TcpSocketContainer> spContainer)
	{
		std::lock_guard<std::mutex> lock(m_MapMutex);
		spContainer->Release();
		m_spClientMap.erase(spContainer);

		
	}
	void TcpClientBase::OnConnected(std::shared_ptr<TcpSocketContainer> spContainer)
	{

		DebugLog("OnConnected TcpClientBase");

	}

}