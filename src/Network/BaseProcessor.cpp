#pragma once 
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "TcpSocketContainer.h"
#include "UdpSocketBase.h"
#include "BaseSocketContainer.h"
#include "../../proto/BaseCmd.pb.h"
#include "BaseMessageDispatcher.h"


namespace DDRFramework
{

	#define REG_ROUTE(cmd,totype,dir) \
	cmd cmd;\
	std::shared_ptr<CommonHeader> sp##cmd = std::make_shared<CommonHeader>();\
	sp##cmd->add_toclttype(totype);\
	sp##cmd->add_flowdirection(dir);\
	m_RouteMsgHeaderMap.insert(make_pair(##cmd.GetTypeName(), sp##cmd));


	MsgRouterManager::MsgRouterManager()
	{

		REG_ROUTE(reqCmdMove, eCltType::eLSMSlamNavigation,CommonHeader_eFlowDir_Forward)
			REG_ROUTE(reqCmdChangeSpeed, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(reqCmdPauseResume, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)


			REG_ROUTE(reqCmdIPC, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(reqCmdAudio, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)


			REG_ROUTE(reqFileAddress, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)



			REG_ROUTE(rspFileAddress, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdMove, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
	}

	std::shared_ptr<DDRCommProto::CommonHeader> MsgRouterManager::FindCommonHeader(std::string bodytype)
	{
		if (m_RouteMsgHeaderMap.find(bodytype) != m_RouteMsgHeaderMap.end())
		{
			return m_RouteMsgHeaderMap[bodytype];
		}
		return nullptr;
	}

}