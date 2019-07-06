#include "LocalServerHeadRuleRouter.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/repeated_field.h>


LocalServerHeadRuleRouter::LocalServerHeadRuleRouter()
{
}


LocalServerHeadRuleRouter::~LocalServerHeadRuleRouter()
{
}

bool LocalServerHeadRuleRouter::IgnoreBody(std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, asio::streambuf& buf, int bodylen)
{
	if (spHeader->flowdirection().size() > 0)
	{
		//if forward , record the route node
		if (spHeader->flowdirection(0) == CommonHeader_eFlowDir_Forward)
		{

			eCltType toType = spHeader->toclttype();

			auto spSession = spSockContainer->GetTcp();
			MsgRouterManager::Instance()->RecordPassNode(spHeader, spSession);


			bool hasSession = false;

			//do Local forward operation
			auto map = GlobalManager::Instance()->GetTcpServer()->GetTcpSocketContainerMap();

			for (auto pair : map)
			{

				auto spSession = pair.second;
				auto spServerSessionTcp = dynamic_pointer_cast<LocalServerTcpSession>(spSession);

				auto sessiontype = spServerSessionTcp->GetLoginInfo().type();
				if ((sessiontype & toType) != 0)
				{
					spServerSessionTcp->Send(spHeader, buf, bodylen);
					hasSession = true;
				}
			}

			if (hasSession == false)
			{
				LevelLog(DDRFramework::Log::Level::INFO,"No Dest Session Conncected:%i", toType);
			}

			//Client Session Operation(To Remote Server)
			if ((eAllClient & toType) != 0)
			{
				auto spTcp = LSClientManager::Instance()->GetTcpClient();
				if (spTcp)
				{
					auto spClientSession = spTcp->GetConnectedSession();
					if (spClientSession)
					{
						spClientSession->Send(spHeader, buf, bodylen);
					}

				}
			}

			return true;
		}
		else if (spHeader->flowdirection(0) == CommonHeader_eFlowDir_Backward)
		{

			CommonHeader_PassNode passnode;
			if (MsgRouterManager::Instance()->ReturnPassNode(spHeader, passnode))
			{

				//Client Session Operation(To Remote Server)
				auto spTcp = LSClientManager::Instance()->GetTcpClient();
				if (spTcp)
				{
					auto spClientSession = spTcp->GetConnectedSession();
					if (spClientSession)
					{
						size_t IntPtr = (size_t)(spClientSession.get());
						if (passnode.nodetype() == eLocalServer)
						{
							if (IntPtr == passnode.receivesessionid())
							{
								spClientSession->Send(spHeader, buf, bodylen);
							}
						}
					}

				}


				//Server Session Operation;
				auto map = GlobalManager::Instance()->GetTcpServer()->GetTcpSocketContainerMap();
				std::shared_ptr<TcpSessionBase> spSession = nullptr;

				for (auto spSessionPair : map)
				{
					size_t IntPtr = (size_t)(spSessionPair.second.get());
					if (passnode.nodetype() == eLocalServer)
					{
						if (IntPtr == passnode.receivesessionid())
						{
							spSession = spSessionPair.second;
							spSession->Send(spHeader, buf, bodylen);
							break;
						}
					}
				}


			}
			else
			{
				LevelLog(DDRFramework::Log::Level::ERR,"Router Info from CommonHeader Error");
			}

			return true;
		}


		return false;
	}
	else
	{
		return false;
	}

};