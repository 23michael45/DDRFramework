#include "RouteServer.h"

namespace DDRCloudService {

static const int s_SessionRcvBufSz = 512 * 1024;

ClientTcpSession::ClientTcpSession(asio::io_context& context)
	: TcpSessionBase(context)
{
	GetRecvBuf().prepare(s_SessionRcvBufSz);
}

ClientTcpSession::~ClientTcpSession()
{
	LevelLog(DDRFramework::Log::Level::INFO, "LocalServerTcpSession Destroy");
}

void ClientTcpSession::OnStart()
{
	TcpSessionBase::OnStart();
	auto spClientBehavior = std::make_shared<BaseClientBehavior>();
	BindBehavior(spClientBehavior);
}


RouteServer::RouteServer(int port) :TcpServerBase(port)
{
}

RouteServer::~RouteServer()
{
}

std::shared_ptr<TcpSessionBase> RouteServer::BindSerializerDispatcher()
{
	auto pTcpCltSess = std::make_shared<ClientTcpSession>(m_IOContext);
	auto pSerializer = std::make_shared<MessageSerializer>();
	pTcpCltSess->LoadSerializer(pSerializer);
	auto pDisp = std::make_shared<RouteServerDispatcher>();
	auto pHeadRuleRouter = std::make_shared<BaseHeadRuleRouter>();
	pSerializer->Init();
	pSerializer->BindDispatcher(pDisp, pHeadRuleRouter);
	return pTcpCltSess;
}

}