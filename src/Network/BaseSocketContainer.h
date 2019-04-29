#ifndef BaseSocketContainer_h__
#define BaseSocketContainer_h__
#include "thirdparty/asio/include/asio.hpp"
#include "src/Network/TcpSocketContainer.h"
#include "src/Network/UdpSocketBase.h"

namespace DDRFramework
{
	class TcpSocketContainer;
	class UdpSocketBase;




	std::vector<std::string> GetLocalIPV4();
	std::map<std::string, std::string> GetSameSegmentIPV4(std::vector<std::string> left, std::vector<std::string> right);


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

		std::shared_ptr<TcpSocketContainer>  GetTcp();
		std::shared_ptr<UdpSocketBase> GetUdp();


		asio::io_context& GetIOContext();

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		void SendBack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);

		void PrintRemoteIP(std::string btype);


	private:
		ESOCKETTYPE m_SocketType;
		std::shared_ptr<TcpSocketContainer> m_spTcpSocketContainer;
		std::shared_ptr<UdpSocketBase> m_spUdpSocketBase;
	};

}
#endif // BaseSocketContainer_h__
