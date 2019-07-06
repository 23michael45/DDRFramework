#ifndef __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__
#define __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__

#include <atomic>
#include "src/Network/BaseMessageDispatcher.h"
#include "src/Network/BaseSocketContainer.h"
#include "RobotSideManager.h"

namespace DDRCloudService {

class RUDispatcher : public DDRFramework::BaseMessageDispatcher
{
public:
	RUDispatcher(std::shared_ptr<RobotSideRouteManager> spRouteMgt,
		         std::atomic<__int64> *pLastRcvTic // in seconds since EPOCH
	             );
	~RUDispatcher();

	void Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
		          std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
		          std::shared_ptr<google::protobuf::Message> spMsg) override;
private:
	std::shared_ptr<RobotSideRouteManager> m_spRouteMgt;
	std::atomic<__int64> *m_pLastTic;
};

}

#endif // __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__
