#include "BaseSocketContainer.h"
#include "../Utility/DDRMacro.h"
#include "../Utility/Logger.h"

namespace DDRFramework
{
	std::vector<std::string> GetLocalIPV4()
	{

		std::vector<std::string> vec;

		try
		{
			asio::io_service io_service;
			asio::ip::tcp::resolver resolver(io_service);
			asio::ip::tcp::resolver::query query(asio::ip::host_name(), "");
			asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
			asio::ip::tcp::resolver::iterator end; // End marker.


			while (iter != end)
			{
				asio::ip::tcp::endpoint ep = *iter++;

				if (ep.address().is_v4())
				{
					vec.push_back(ep.address().to_string());
				}
			}
		}
		catch (std::exception& e)
		{

		}
		return vec;

	}


	std::map<std::string, std::string> GetSameSegmentIPV4(std::vector<std::string> left, std::vector<std::string> right)
	{
		std::map<std::string, std::string> rmap;
		for (auto l : left)
		{
			for (auto r : right)
			{
				try
				{
					auto laddr = asio::ip::address_v4::from_string(l);
					auto raddr = asio::ip::address_v4::from_string(r);
					auto mask = asio::ip::address_v4::from_string("255.255.255.0");

					if ((laddr.to_uint() & mask.to_uint()) == (raddr.to_uint() & mask.to_uint()))
					{
						rmap.insert(std::make_pair(laddr.to_string(), raddr.to_string()));
					}


				}
				catch (std::exception& e)
				{
				}

			}
		}
		return rmap;
	}


	BaseSocketContainer::BaseSocketContainer(std::shared_ptr<TcpSocketContainer> sp)
	{
		SetTcp(sp);
	}
	BaseSocketContainer::BaseSocketContainer(std::shared_ptr<UdpSocketBase> sp)
	{
		SetUdp(sp);
	}
	BaseSocketContainer::~BaseSocketContainer()
	{
		if (m_spTcpSocketContainer)
		{
			m_spTcpSocketContainer.reset();
		}

		if (m_spUdpSocketBase)
		{
			m_spUdpSocketBase.reset();
		}
		DebugLog("BaseSocketContainer Destroy");
	}

	void BaseSocketContainer::SetTcp(std::shared_ptr<TcpSocketContainer> sp)
	{
		m_spTcpSocketContainer = sp;
		m_SocketType = ESOCKETTYPE::EST_TCP;
	}
	void BaseSocketContainer::SetUdp(std::shared_ptr<UdpSocketBase> sp)
	{
		m_spUdpSocketBase = sp;
		m_SocketType = ESOCKETTYPE::EST_UDP;
	}
	asio::io_context& BaseSocketContainer::GetIOContext()
	{
		if (m_SocketType == EST_TCP)
		{
			return m_spTcpSocketContainer->GetIOContext();
		}
		else if (m_SocketType == EST_UDP)
		{
			return m_spUdpSocketBase->GetIOContext();
		}
	}

	void BaseSocketContainer::Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_SocketType == EST_TCP)
		{
			m_spTcpSocketContainer->Send(spheader,spmsg);
		}
		else if (m_SocketType == EST_UDP)
		{
			m_spUdpSocketBase->Send(spmsg);
		}

	}
	void BaseSocketContainer::SendBack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg)
	{
		if (m_SocketType == EST_TCP)
		{
			m_spTcpSocketContainer->SendBack(spheader, spmsg);
		}
	}
	void BaseSocketContainer::Send(std::shared_ptr<google::protobuf::Message> spmsg)
	{
		Send(nullptr, spmsg);
	}


	std::shared_ptr<TcpSocketContainer>  BaseSocketContainer::GetTcp()
	{
		return m_spTcpSocketContainer;
	}
	std::shared_ptr<UdpSocketBase> BaseSocketContainer::GetUdp()
	{
		return m_spUdpSocketBase;
	}

	void BaseSocketContainer::PrintRemoteIP(std::string btype)
	{
		try
		{
			if (GetTcp())
			{
				DebugLog("\nReceive TCP Message %s from:%s", btype.c_str(),GetTcp()->GetSocket().remote_endpoint().address().to_string().c_str());

			}
			else if (GetUdp())
			{

				//DebugLog("\nReceive UDP Message %s from:%s", btype.c_str(), GetUdp()->GetRecvSocket()->remote_endpoint().address().to_string().c_str());
				DebugLog("\nReceive UDP Message %s ", btype.c_str())
			}
		}
		catch (asio::error_code& e)
		{

			DebugLog("\nPrintRemoteIP ASIO %s", e.message().c_str())
		}
		catch (std::exception& e)
		{
			DebugLog("\nPrintRemoteIP %s", e.what())
		}
	}
}