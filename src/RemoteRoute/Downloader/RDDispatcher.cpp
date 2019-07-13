#include "RDDispatcher.h"

#include <chrono>
#include "src/Utility/LoggerDef.h"

namespace DDRCloudService {

RDDispatcher::RDDispatcher(std::shared_ptr<DownloadManagerBase> spDMgt,
	                       std::atomic<__int64> *pLastRcvTic)
	: m_spDMgt(spDMgt), m_pLastTic(pLastRcvTic)
{
}

RDDispatcher::~RDDispatcher()
{
}

void RDDispatcher::Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
	                        std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
	                        std::shared_ptr<google::protobuf::Message> spMsg)
{
	if (m_spDMgt) {
		*m_pLastTic = std::chrono::duration_cast<std::chrono::seconds>
			          (std::chrono::system_clock::now().time_since_epoch()).count();
		auto pRsp = m_spDMgt->Feed(spMsg);
		if (pRsp) {
			spParentSocketContainer->SendBack(spHeader, pRsp);
		}
	}
}

}
