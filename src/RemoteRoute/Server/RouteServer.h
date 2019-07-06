#ifndef __DDR_CLOUD_SERVICE_LIB_SERVER_ROUTE_SERVER_H_INCLUDED__
#define __DDR_CLOUD_SERVICE_LIB_SERVER_ROUTE_SERVER_H_INCLUDED__

#include "src/Network/TcpServerBase.h"
#include "src/Network/BaseMessageDispatcher.h"
#include "src/Utility/Singleton.h"
using namespace DDRFramework;
#include "BaseClientBehavior.h"
#include "ServerSideManager.h"

namespace DDRCloudService {

class ClientTcpSession : public TcpSessionBase
{
public:
	ClientTcpSession(asio::io_context& context);
	~ClientTcpSession();
	virtual void OnStart() override;
	SHARED_FROM_BASE(ClientTcpSession)
};

class RouteServerDispatcher : public BaseMessageDispatcher
{
public:
	void Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
		          std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
		          std::shared_ptr<google::protobuf::Message> spMsg) override
	{
		std::dynamic_pointer_cast<BaseClientBehavior>(spParentSocketContainer->GetTcp()->GetBehavior())->ResetHeartBeat();
		auto pRsp = ServerSideRouteManager::Instance()->Feed(spMsg);
		if (pRsp) {
			spParentSocketContainer->SendBack(spHeader, pRsp);
		}
	}
};

class RouteServer : public TcpServerBase
{
public:
	RouteServer(int port);
	~RouteServer();
protected:
	std::shared_ptr<TcpSessionBase> BindSerializerDispatcher() override;
	SHARED_FROM_BASE(RouteServer)
};



}

#endif // __DDR_CLOUD_SERVICE_LIB_SERVER_ROUTE_SERVER_H_INCLUDED__
