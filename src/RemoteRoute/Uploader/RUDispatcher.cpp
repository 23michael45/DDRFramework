#include "RUDispatcher.h"

#include <chrono>
#include "src/Utility/LoggerDef.h"

namespace DDRCloudService {

RUDispatcher::RUDispatcher(std::shared_ptr<RobotSideRouteManager> spRouteMgt,
	                       std::atomic<long long> *pLastRcvTic)
	: m_spRouteMgt(spRouteMgt), m_pLastTic(pLastRcvTic)
{
}

RUDispatcher::~RUDispatcher()
{
}

void RUDispatcher::Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
	                        std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
	                        std::shared_ptr<google::protobuf::Message> spMsg)
{
	if (m_spRouteMgt) {
		*m_pLastTic = std::chrono::duration_cast<std::chrono::seconds>
			          (std::chrono::system_clock::now().time_since_epoch()).count();
		auto pRsp = m_spRouteMgt->Feed(spMsg);
		if (pRsp) {
			spParentSocketContainer->SendBack(spHeader, pRsp);
		}
	}
}

}
