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
		REG_ROUTE(reqCmdUpdateConfig, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdGetConfigdata, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdSetWorkPath, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdGeneralOctree, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdStartActionMode, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdEndActionMode, eCltType::eLSMSlamNavigation, CommonHeader_eFlowDir_Forward)

		REG_ROUTE(reqCmdIPC, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdAudio, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdAutoChat, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)

		REG_ROUTE(reqCmdAddFace, eCltType::eLSMFaceRecognition, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdFaceDataBaseOrder, eCltType::eLSMFaceRecognition, CommonHeader_eFlowDir_Forward)
		REG_ROUTE(reqCmdSetFaceParams, eCltType::eLSMFaceRecognition, CommonHeader_eFlowDir_Forward)

		REG_ROUTE(reqCmdGetAlarmParams, eCltType::eAllLSM, CommonHeader_eFlowDir_Forward)
		
		REG_ROUTE(reqFileAddress, eCltType::eLSMStreamRelay, CommonHeader_eFlowDir_Forward)


		REG_ROUTE(rspCmdMove, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdChangeSpeed, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdPauseResume, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdUpdateConfig, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdGetConfigdata, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdSetWorkPath, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdGeneralOctree, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdStartActionMode, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdEndActionMode, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)

			REG_ROUTE(rspCmdIPC, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdAudio, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdAutoChat, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)

			REG_ROUTE(rspCmdAddFace, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdFaceDataBaseOrder, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspCmdSetFaceParams, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)

			REG_ROUTE(rspCmdGetAlarmParams, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)
			REG_ROUTE(rspFileAddress, eCltType::eAllClient, CommonHeader_eFlowDir_Backward)




			REG_ROUTE(notifyBaseStatus, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyObsData, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyAlarmInfo, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyAlarmNewFace, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyAlarmFire, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyStreamAddr, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyAlarm, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)
			REG_ROUTE(notifyStreamServiceInfoChanged, eCltType::eAllClient, CommonHeader_eFlowDir_Forward)


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