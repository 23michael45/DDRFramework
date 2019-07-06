#include "LocalServerDispatcher.h"

#include "LocalTcpServer.h"


#include "src/Utility/DDRMacro.h"

using namespace DDRCommProto;
//using namespace DDRModuleProto;
using namespace DDRFramework;

LocalServerDispatcher::LocalServerDispatcher()
{
	HeartBeat hBeat;
	m_ProcessorMap[hBeat.GetTypeName()] = std::make_shared<HeartBeatProcessor>(*this);

}\


LocalServerDispatcher::~LocalServerDispatcher()
{
}

void LocalServerDispatcher::Dispatch(std::shared_ptr< DDRFramework::BaseSocketContainer> spParentSocketContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
{
	auto spServerTcpSession = dynamic_pointer_cast<LocalServerTcpSession>(spParentSocketContainer->GetTcp());

	if (spServerTcpSession->HasLogin() || spHeader->bodytype() == "DDRCommProto.reqLogin")
	{

		BaseMessageDispatcher::Dispatch(spParentSocketContainer, spHeader, spMsg);
	}
	else
	{
		LevelLog(DDRFramework::Log::Level::ERR,"Dispatch Error , has not login")
	}

}
