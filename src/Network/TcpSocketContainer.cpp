#include "TcpSocketContainer.h"
#include "MessageSerializer.h"
#include "../Utility/Logger.h"

#include "asio.hpp"
#include <map>
#include <chrono>
#include <thread>
using asio::ip::tcp;

namespace DDRFramework
{
	TcpSocketContainer::TcpSocketContainer(asio::io_context &context) : m_IOContext(context), m_Socket(context), m_bConnected(false), m_ReadWriteStrand(m_IOContext)
	{
		SetRealtime(false);
	}
	TcpSocketContainer::~TcpSocketContainer()
	{
		LevelLog(DDRFramework::Log::Level::INFO,"TcpSocketContainer Destroy");
		Release();
	}
	void  TcpSocketContainer::Start()
	{
		OnStart();
		m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckRead, shared_from_this()));
		m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));


		auto ip = GetSocket().remote_endpoint().address().to_string();
		m_fromIP = ip;
	}
	void TcpSocketContainer::CheckBehavior()
	{
		if (m_bConnected)
		{
			if (m_spBehavior)
			{
				m_spBehavior->Update(shared_from_this());
				m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckBehavior, shared_from_this()));
			}
		}

	}

	void TcpSocketContainer::CheckRead()
	{
		if (m_bConnected)
		{
			if (m_spSerializer)
			{
				m_spSerializer->Update();
			}
			if (m_iCheckReadSleep > 0)
			{
				std::this_thread::sleep_for(chrono::milliseconds(m_iCheckReadSleep));

			}
			m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckRead, shared_from_this()));
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"\CheckRead Not Connected");
		}
	}
	void TcpSocketContainer::CheckWrite()
	{
		if (m_spSerializer)
		{
			std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());
			if (m_bConnected)
			{
				auto spbuf = m_spSerializer->GetSendBuf();
				if (spbuf)
				{
					StartWrite(spbuf);
				}
				else
				{
					if (m_iCheckWriteSleep > 0)
					{
						std::this_thread::sleep_for(chrono::milliseconds(m_iCheckWriteSleep));

					}
					m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));
				}

			}
		}
	}

	void TcpSocketContainer::PushData(asio::streambuf& buf)
	{
		if (m_bConnected)
		{
			std::lock_guard<std::mutex> lock(m_spSerializer->GetRecLock());

			if (m_fOnHookReceiveData)
			{
				m_fOnHookReceiveData(buf);
			}
			else
			{

				if (m_spSerializer)
				{
					std::ostream oshold(&m_spSerializer->GetRecBuf());

					oshold.write((const char*)buf.data().data(), buf.size());
					oshold.flush();

				}

			}
			buf.consume(buf.size());

		}
	}

	void TcpSocketContainer::SendBack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{
				if(spheader->flowdirection_size() > 0)
				{
					spheader->set_flowdirection(0, CommonHeader_eFlowDir_Backward);

				}
				m_spSerializer->Pack(spheader, spmsg);

			}
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"Disconnected Send Failed");

		}

	}
	void TcpSocketContainer::Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{
				m_spSerializer->Pack(spheader, spmsg);

			}
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"Disconnected Send Failed");

		}

	}
	void TcpSocketContainer::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		Send(nullptr, spmsg);
	}

	void TcpSocketContainer::Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, asio::streambuf& buf, int bodylen)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{
				m_spSerializer->Pack(spheader, buf, bodylen);

			}
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"Disconnected Send Failed");

		}
	}
	void TcpSocketContainer::Send(std::shared_ptr<asio::streambuf> spbuf)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{
				std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());


				//create a new buf ,cause write will cosume the buf,this old buf maybe used by another session
				std::shared_ptr<asio::streambuf> spbufnew = std::make_shared<asio::streambuf>();

				std::ostream oshold(spbufnew.get());
				oshold.write((const char*)spbuf->data().data(), spbuf->size());
				oshold.flush();


				m_spSerializer->PushSendBuf(spbufnew);

			}
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"Disconnected Send Failed");

		}
	}
	void TcpSocketContainer::Send(const void* psrc, int len)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{

				std::shared_ptr<asio::streambuf> spbuf = std::make_shared<asio::streambuf>();
				std::ostream oshold(spbuf.get());

				oshold.write((const char*)psrc, len);
				oshold.flush();

				std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());
				m_spSerializer->PushSendBuf(spbuf);

			}
		}
		else
		{
			LevelLog(DDRFramework::Log::Level::WARNING,"Disconnected Send Failed");

		}

	}
	void TcpSocketContainer::HandleWrite(const asio::error_code& ec, size_t size)
	{
		if (m_spSerializer)
		{
			std::lock_guard<std::mutex> lock(m_spSerializer->GetSendLock());
			if (m_bConnected)
			{
				if (!ec)
				{
					m_spSerializer->PopSendBuf();
					m_CurrentWritingBuf.reset();
				}
				else
				{
					LevelLog(DDRFramework::Log::Level::ERR,"TcpSocketContainer Error on send:  %s" , ec.message().c_str());

					Stop();

				}

				m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));

			}
		}

	}

	tcp::socket& TcpSocketContainer::GetSocket()
	{
		return m_Socket;
	}
	void TcpSocketContainer::Stop()
	{
		//do not need bConnect,cause some Seesion have not connect but still need relase
		//if (m_bConnected)  
		//{
			m_bConnected = false;


			//on stop must before callOnDisconnect;cause OnStop will delete session in map,then call ondisconnect will destroy session.  if destroy session before delete ,crash may occur
			OnStop();

			m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CallOnDisconnect, shared_from_this()));//cannot stop twice so we judge bConnected first,otherwiese it will call CallOnDisconnect twice,maybe on called this time ,and io_context pause,and the other will be call next time when the io_context resume


		//}
	}

	std::shared_ptr<MessageSerializer> TcpSocketContainer::GetSerializer()
	{
		return m_spSerializer;
	}

	void TcpSocketContainer::LoadSerializer(std::shared_ptr<MessageSerializer> sp)
	{
		m_spSerializer = sp;
		m_spSerializer->BindBaseSocketContainer(std::make_shared<BaseSocketContainer>(shared_from_this()));
	}
	void TcpSocketContainer::Release()
	{
		if (m_spSerializer)
		{
			m_spSerializer.reset();
		}
		if (m_spBehavior)
		{
			m_spBehavior->OnStop(shared_from_this());
			m_spBehavior.reset();

		}
		m_Socket.close();
	}
	void TcpSocketContainer::CallOnDisconnect()
	{
		if (m_fOnSessionDisconnect)
		{
			m_fOnSessionDisconnect(shared_from_this());
		}



		BindOnConnected(nullptr);
		BindOnDisconnect(nullptr);
		BindOnConnectTimeout(nullptr);
		BindOnConnectFailed(nullptr);
	}

	void TcpSocketContainer::BindBehavior(std::shared_ptr<BaseBehavior> behavior)
	{
		if (m_spBehavior)
		{
			m_spBehavior->OnStop(shared_from_this());
		}
		m_spBehavior = behavior;

		if (m_spBehavior)
		{
			m_spBehavior->OnStart(shared_from_this());
			m_ReadWriteStrand.post(std::bind(&TcpSocketContainer::CheckBehavior, shared_from_this()));
		}
	}

	std::string TcpSocketContainer::GetIPAddress()
	{
		if (m_Socket.is_open())
		{
			//return m_Socket.remote_endpoint().address().to_string();
		
			return m_fromIP;
		}
	}

	void TcpSocketContainer::SetRealtime(bool b)
	{
		if (b)
		{
			m_iCheckReadSleep = 0;
			m_iCheckWriteSleep = 0;

		}
		else
		{

			m_iCheckReadSleep = 1;
			m_iCheckWriteSleep = 1;
		}
	}

	bool TcpSocketContainer::RegisterExternalProcessor(google::protobuf::Message& msg, std::shared_ptr<BaseProcessor> sp)
	{
		if (m_spSerializer)
		{
			return m_spSerializer->RegisterExternalProcessor(msg, sp);
		}

		return false;
	}

	bool TcpSocketContainer::UnregisterExternalProcessor(google::protobuf::Message& msg)
	{
		if (m_spSerializer)
		{
			return m_spSerializer->UnregisterExternalProcessor(msg);
		}

		return false;
	}

}
