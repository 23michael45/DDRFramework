#include "TcpSocketContainer.h"
#include "MessageSerializer.h"
#include "../Utility/DDRMacro.h"

#include "asio.hpp"
#include <map>
using asio::ip::tcp;

namespace DDRFramework
{
	TcpSocketContainer::TcpSocketContainer(asio::io_context &context) : m_IOContext(context), m_Socket(context), m_bConnected(false)
	{

	}
	TcpSocketContainer::~TcpSocketContainer()
	{
		m_spSerializer.reset();
	}
	void  TcpSocketContainer::Start()
	{
		m_IOContext.post(std::bind(&TcpSocketContainer::Update, shared_from_this()));
	}

	void TcpSocketContainer::Update()
	{
		if (m_bConnected)
		{
			if (m_spSerializer)
			{
				m_spSerializer->Update();

			}
			m_IOContext.post(std::bind(&TcpSocketContainer::Update, shared_from_this()));
		}
		else
		{
			DebugLog("\nUpdate Finish");
		}
	}
	void TcpSocketContainer::Send(google::protobuf::Message& msg)
	{
		if (m_bConnected)
		{
			m_spSerializer->Pack(msg);

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

	std::shared_ptr<MessageSerializer> TcpSocketContainer::GetSerializer()
	{
		return m_spSerializer;
	}

	void TcpSocketContainer::LoadSerializer(std::shared_ptr<MessageSerializer> sp)
	{
		m_spSerializer = sp;
		m_spSerializer->BindSendFunction(std::bind(&TcpSocketContainer::StartWrite, shared_from_this(),std::placeholders::_1));
	}
	void TcpSocketContainer::UnloadSerializer()
	{
		m_spSerializer->Deinit();
		m_spSerializer.reset();
	}

}

