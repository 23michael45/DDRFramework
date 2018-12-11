#include "TcpSocketContainer.h"
#include "MessageSerializer.h"
#include "../Utility/DDRMacro.h"

#include "asio.hpp"
#include <map>
using asio::ip::tcp;

namespace DDRFramework
{
	TcpSocketContainer::TcpSocketContainer(asio::io_context &context) : m_IOContext(context), m_Socket(context), m_bConnected(false), m_WriteStrand(m_IOContext),m_ReadStrand(m_IOContext)
	{

	}
	TcpSocketContainer::~TcpSocketContainer()
	{
		DebugLog("\nTcpSocketContainer Destroy");
		m_spSerializer.reset();
	}
	void  TcpSocketContainer::Start()
	{
		m_ReadStrand.post(std::bind(&TcpSocketContainer::CheckRead, shared_from_this()));
		m_WriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));
	}

	void TcpSocketContainer::CheckRead()
	{
		if (m_bConnected)
		{
			if (m_spSerializer)
			{
				m_spSerializer->Update();
			}
			m_WriteStrand.post(std::bind(&TcpSocketContainer::CheckRead, shared_from_this()));
		}
		else
		{
			DebugLog("\CheckRead Finish");
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

					m_WriteStrand.post(std::bind(&TcpSocketContainer::CheckWrite, shared_from_this()));
				}

			}
		}
	}

	void TcpSocketContainer::PushData(asio::streambuf& buf)
	{
		std::lock_guard<std::mutex> lock(m_spSerializer->GetRecLock());
		if (m_spSerializer)
		{
			std::ostream oshold(&m_spSerializer->GetRecBuf());

			oshold.write((const char*)buf.data().data(), buf.size());
			oshold.flush();

		}
		buf.consume(buf.size());
	}
	void TcpSocketContainer::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_bConnected)
		{
			//m_IOContext.post(std::bind(&MessageSerializer::Pack, m_spSerializer, spmsg));

			//do not use post cause it will block another post StartWrite ,and it's will not do handler function
			if (m_spSerializer)
			{
				m_spSerializer->Pack(spmsg);

			}
		}
		else
		{
			DebugLog("\nDisconnected Send Failed");

		}
	};
	tcp::socket& TcpSocketContainer::GetSocket()
	{
		return m_Socket;
	}
	void TcpSocketContainer::CloseSocket()
	{
		m_bConnected = false;
		m_Socket.close();
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
	void TcpSocketContainer::UnloadSerializer()
	{
		if (m_spSerializer)
		{
			m_spSerializer.reset();
		}
	}
	void TcpSocketContainer::CallOnDisconnect()
	{
		if (m_fOnSessionDisconnect)
		{
			m_fOnSessionDisconnect(*this);
		}
	}

}

