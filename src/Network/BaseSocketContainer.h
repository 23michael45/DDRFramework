#ifndef BaseSocketContainer_h__
#define BaseSocketContainer_h__
#include "../../thirdparty/asio/include/asio.hpp"
#include "TcpSocketContainer.h"
#include "UdpSocketBase.h"

namespace DDRFramework
{
	class TcpSocketContainer;
	class UdpSocketBase;

	class BaseSocketContainer
	{
	public:
		BaseSocketContainer(std::shared_ptr<TcpSocketContainer> sp);
		BaseSocketContainer(std::shared_ptr<UdpSocketBase> sp);
		~BaseSocketContainer();



		enum ESOCKETTYPE
		{
			EST_TCP,
			EST_UDP,
		};

		void SetTcp(std::shared_ptr<TcpSocketContainer> sp);
		void SetUdp(std::shared_ptr<UdpSocketBase> sp);
		ESOCKETTYPE m_SocketType;
		std::shared_ptr<TcpSocketContainer> m_spTcpSocketContainer;
		std::shared_ptr<UdpSocketBase> m_spUdpSocketBase;

		asio::io_context& GetIOContext();

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
	};

}
#endif // BaseSocketContainer_h__
