#include "UdpSocketBase.h"
#include "MessageSerializer.h"
#include <chrono>
#include <thread>

namespace DDRFramework
{
	UdpSocketBase::UdpSocketBase() : m_ReadWriteStrand(m_IOContext)
	{
		m_Broadcasting = false;
		m_Receiving = false;
	}

	UdpSocketBase::~UdpSocketBase()
	{
		DebugLog("UdpBroadcaster Destroy");
	}

	void UdpSocketBase::Start()
	{
		m_spSerializer = std::make_shared<MessageSerializer>();
		m_spSerializer->BindBaseSocketContainer(std::make_shared<BaseSocketContainer>(shared_from_this()));
		m_spSerializer->Init();

		m_spWork = std::make_shared< asio::io_context::work>(m_IOContext);



		std::thread t(std::bind(&UdpSocketBase::ThreadEntry, shared_from_this()));
		t.detach();
	}

	void UdpSocketBase::ThreadEntry()
	{
		m_IOContext.run();
	}

	void UdpSocketBase::Stop()
	{
		if (m_Broadcasting || m_Receiving)
		{
			m_Broadcasting = false;
			m_Receiving = false;
			m_ReadWriteStrand.post(std::bind(&UdpSocketBase::DelayStop, shared_from_this()));

		}
		
	}

	bool UdpSocketBase::IsWorking()
	{
		if (m_Broadcasting || m_Receiving)
		{
			return true;
		}
		return false;
	}
	void UdpSocketBase::DelayStop()
	{
		std::lock_guard<std::mutex> lock(m_UdpMutex);
		m_spRecvEnderEndpoint.reset();
		m_spBroadcastEnderEndpoint.reset();

		m_Broadcasting = false;
		m_Receiving = false;
		m_spWork.reset();
		m_spSerializer.reset();
		if (m_spSendSocket)
		{
			m_spSendSocket->close();
			m_spSendSocket.reset();

		}
		if (m_spRecvSocket)
		{
			m_spRecvSocket->close();
			m_spRecvSocket.reset();

		}
		if (m_fOnDisconnect)
		{
			m_fOnDisconnect(*this);
		}
	}

	void UdpSocketBase::StartBroadcast(int port, std::shared_ptr<google::protobuf::Message> spMsg, int intervalintervalMillisecond)
	{
		m_spBroadcastEnderEndpoint = std::make_shared<asio::ip::udp::endpoint>(asio::ip::address_v4::broadcast(), port);


		m_spSendSocket = std::make_shared< asio::ip::udp::socket>(m_IOContext);// , *(m_spBroadcastEnderEndpoint.get());
		m_spSendSocket->open(asio::ip::udp::v4());
		m_spSendSocket->set_option(asio::ip::udp::socket::reuse_address(true));
		m_spSendSocket->set_option(asio::socket_base::broadcast(true));




		m_Broadcasting = true;
		m_IntervalintervalMillisecond = intervalintervalMillisecond;
		m_spMsg = spMsg;



		auto spbuf = m_spSerializer->SerializeMsg(spMsg);
		m_ReadWriteStrand.post(std::bind(&UdpSocketBase::StartWrite, shared_from_this(), spbuf));
	}
	void UdpSocketBase::StopBroadcast()
	{
		m_Broadcasting = false;
		m_ReadWriteStrand.post(std::bind(&UdpSocketBase::FreeWrite, shared_from_this()));
	}
	void UdpSocketBase::FreeWrite()
	{
		m_spMsg.reset();

	}


	void UdpSocketBase::StartReceive(int port)
	{
		try
		{
			m_ReadWriteStrand.post(std::bind(&UdpSocketBase::DelayStart, shared_from_this(),port));
		}
		catch (asio::system_error& e)
		{
			DebugLog("%s", e.what());
			std::this_thread::sleep_for(std::chrono::seconds(1));
			StopReceive();
		}
	}


	void UdpSocketBase::StopReceive()
	{
		if (m_Receiving)
		{
			m_ReadWriteStrand.post(std::bind(&UdpSocketBase::FreeRead, shared_from_this()));
			m_Receiving = false;

		}
	}
	void UdpSocketBase::FreeRead()
	{
	}

	void UdpSocketBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{
		if (m_Broadcasting)
		{
			m_spSendSocket->async_send_to(spbuf->data(), *(m_spBroadcastEnderEndpoint.get()), std::bind(&UdpSocketBase::HandleWrite, shared_from_this(), std::placeholders::_1, spbuf));

		}

	}
	void UdpSocketBase::HandleWrite(const asio::error_code& ec,std::shared_ptr<asio::streambuf> spbuf) {
	

			if (!ec)
			{
				if (m_Broadcasting)
				{
					std::this_thread::sleep_for(chrono::milliseconds(1));
					m_ReadWriteStrand.post(std::bind(&UdpSocketBase::StartWrite, shared_from_this(), spbuf));
					std::this_thread::sleep_for(std::chrono::milliseconds(m_IntervalintervalMillisecond));

				}
			}
			else
			{
				DebugLog("Udp Write Failed:%s", ec.message());
			}
		
	}

	void UdpSocketBase::StartRead()
	{
		if (m_Receiving)
		{
			if (m_spRecvSocket)
			{
				m_spRecvSocket->async_receive(asio::buffer(m_ReadStreamBuf), std::bind(&UdpSocketBase::HandleRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));

			}
			else
			{
				std::this_thread::sleep_for(chrono::milliseconds(1));
				m_ReadWriteStrand.post(std::bind(&UdpSocketBase::StartRead, shared_from_this()));

			}
		}

		/*asio::ip::udp::endpoint sender_endpoint(asio::ip::udp::v4(), 7000);

		asio::ip::udp::socket socket(m_IOContext, sender_endpoint);
		socket.open(asio::ip::udp::v4());

		char packet[1024];

		asio::error_code error;
		size_t len = socket.receive_from(asio::buffer(packet), sender_endpoint);*/


	}

	void UdpSocketBase::DelayStart(int port)
	{

		m_Receiving = true;
		m_spRecvEnderEndpoint = std::make_shared<asio::ip::udp::endpoint>(asio::ip::address_v4::any(), port);
		//m_spSocket = std::make_shared< asio::ip::udp::socket>(m_IOContext), *(m_spRecvEnderEndpoint.get());

		m_spRecvSocket = std::make_shared<asio::ip::udp::socket>(m_IOContext);

		m_spRecvSocket->open(m_spRecvEnderEndpoint->protocol());
		m_spRecvSocket->set_option(asio::ip::udp::socket::reuse_address(true));
		m_spRecvSocket->bind(*m_spRecvEnderEndpoint.get());

		m_ReadWriteStrand.post(std::bind(&UdpSocketBase::StartRead, shared_from_this()));
	}

	void UdpSocketBase::HandleRead(const asio::error_code& ec, int len)
	{
		std::lock_guard<std::mutex> lock(m_UdpMutex);
		if (m_Receiving)
		{

			if (!ec)
			{
				if (m_fOnHookReceiveData)
				{
					m_fOnHookReceiveData(m_ReadStreamBuf,len);
				}
				else
				{
					if (m_spSerializer)
					{

						if (m_spSerializer)
						{
							std::ostream oshold(&m_spSerializer->GetRecBuf());

							oshold.write(m_ReadStreamBuf.data(), len);
							oshold.flush();

						}

						m_spSerializer->Update();
					}
				}

				if (m_Receiving)
				{
					std::this_thread::sleep_for(chrono::milliseconds(1));
					m_ReadWriteStrand.post(std::bind(&UdpSocketBase::StartRead, shared_from_this()));
				}
			}
			else
			{
				DebugLog("Udp Read Failed:%s", ec.message().c_str());
			}
		}
	}

	void UdpSocketBase::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		//m_Socket.async_send_to(spbuf->data(), *(m_spEnderEndpoint.get()), std::bind(&UdpSocketBase::HandleWrite, shared_from_this(), std::placeholders::_1, spbuf));
	}
	asio::io_context& UdpSocketBase::GetIOContext()
	{
		return m_IOContext;
	}
}