#include "UdpSocketBase.h"
#include "MessageSerializer.h"
#include <chrono>
#include <thread>

namespace DDRFramework
{
	UdpSocketBase::UdpSocketBase() : m_WriteStrand(m_IOContext), m_ReadStrand(m_IOContext)
	{
		m_Broadcasting = false;
		m_Receiving = false;
	}

	UdpSocketBase::~UdpSocketBase()
	{
		DebugLog("\nUdpBroadcaster Destroy");
	}

	void UdpSocketBase::Start()
	{
		m_spSerializer = std::make_shared<MessageSerializer>();
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
		m_spRecvEnderEndpoint.reset(); 
		m_spBroadcastEnderEndpoint.reset();

		m_Broadcasting = false;
		m_Receiving = false;
		m_spWork.reset();
		m_spSerializer.reset();
		m_spSocket->close();
		m_spSocket.reset();
	}

	void UdpSocketBase::StartBroadcast(int port, std::shared_ptr<google::protobuf::Message> spMsg, int intervalintervalMillisecond)
	{
		m_spBroadcastEnderEndpoint = std::make_shared<asio::ip::udp::endpoint>(asio::ip::address_v4::broadcast(), port);


		m_spSocket = std::make_shared< asio::ip::udp::socket>(m_IOContext);// , *(m_spBroadcastEnderEndpoint.get());
		m_spSocket->open(asio::ip::udp::v4());
		m_spSocket->set_option(asio::ip::udp::socket::reuse_address(true));
		m_spSocket->set_option(asio::socket_base::broadcast(true));




		m_Broadcasting = true;
		m_IntervalintervalMillisecond = intervalintervalMillisecond;
		m_spMsg = spMsg;



		auto spbuf = m_spSerializer->SerlializeMsg(spMsg);
		m_WriteStrand.post(std::bind(&UdpSocketBase::StartWrite, shared_from_this(), spbuf));
	}
	void UdpSocketBase::StopBroadcast()
	{
		m_Broadcasting = false;
		m_WriteStrand.post(std::bind(&UdpSocketBase::FreeWrite, shared_from_this()));
	}
	void UdpSocketBase::FreeWrite()
	{
		m_spMsg.reset();

	}


	void UdpSocketBase::StartReceive(int port)
	{
		m_Receiving = true;
		m_spRecvEnderEndpoint = std::make_shared<asio::ip::udp::endpoint>(asio::ip::address_v4::any(), port);
		//m_spSocket = std::make_shared< asio::ip::udp::socket>(m_IOContext), *(m_spRecvEnderEndpoint.get());
		m_spSocket = std::make_shared<asio::ip::udp::socket>(m_IOContext, *(m_spRecvEnderEndpoint.get()));
		//m_spSocket->open(m_spRecvEnderEndpoint->protocol());


		m_ReadStrand.post(std::bind(&UdpSocketBase::StartRead, shared_from_this()));
	}
	void UdpSocketBase::StopReceive()
	{
		m_ReadStrand.post(std::bind(&UdpSocketBase::FreeRead, shared_from_this()));
		m_Receiving = false;
	}
	void UdpSocketBase::FreeRead()
	{
		m_ReadStreamBuf.consume(m_ReadStreamBuf.size());
	}

	void UdpSocketBase::StartWrite(std::shared_ptr<asio::streambuf> spbuf)
	{
		m_spSocket->async_send_to(spbuf->data(),*(m_spBroadcastEnderEndpoint.get()), std::bind(&UdpSocketBase::HandleWrite, shared_from_this(), std::placeholders::_1,spbuf));


	}
	void UdpSocketBase::HandleWrite(const asio::error_code& ec,std::shared_ptr<asio::streambuf> spbuf) {
		
		std::this_thread::sleep_for(std::chrono::milliseconds(m_IntervalintervalMillisecond));

		if (!ec)
		{
			if (m_Broadcasting)
			{
				m_WriteStrand.post(std::bind(&UdpSocketBase::StartWrite, shared_from_this(), spbuf));
			}
		}
		else
		{
			DebugLog("\nUdp Write Failed:%s", ec.message());
		}
	}

	void UdpSocketBase::StartRead()
	{
		m_spSocket->async_receive_from(asio::buffer(m_ReadStreamBuf.prepare(1024)), *(m_spRecvEnderEndpoint.get()) ,std::bind(&UdpSocketBase::HandleRead, shared_from_this(), std::placeholders::_1));


		/*asio::ip::udp::endpoint sender_endpoint(asio::ip::udp::v4(), 7000);

		asio::ip::udp::socket socket(m_IOContext, sender_endpoint);
		socket.open(asio::ip::udp::v4());

		char packet[1024];

		asio::error_code error;
		size_t len = socket.receive_from(asio::buffer(packet), sender_endpoint);*/


	}
	void UdpSocketBase::HandleRead(const asio::error_code& ec)
	{
		if (!ec)
		{
			if (m_spSerializer)
			{
				std::lock_guard<std::mutex> lock(m_spSerializer->GetRecLock());
				if (m_spSerializer)
				{
					std::ostream oshold(&m_spSerializer->GetRecBuf());

					oshold.write((const char*)m_ReadStreamBuf.data().data(), m_ReadStreamBuf.size());
					oshold.flush();

				}
				m_ReadStreamBuf.consume(m_ReadStreamBuf.size());

				m_spSerializer->Update();
			}

			if (m_Receiving)
			{
				m_ReadStrand.post(std::bind(&UdpSocketBase::StartRead, shared_from_this()));
			}
		}
		else
		{
			DebugLog("\nUdp Read Failed:%s", ec.message().c_str());
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