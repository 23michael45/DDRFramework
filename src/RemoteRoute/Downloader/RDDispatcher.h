#ifndef __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__
#define __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__

#include <atomic>
#include "src/Network/BaseMessageDispatcher.h"
#include "src/Network/BaseSocketContainer.h"
#include "DownloadManagerBase.h"

namespace DDRCloudService {

class RDDispatcher : public DDRFramework::BaseMessageDispatcher
{
public:
	RDDispatcher(std::shared_ptr<DownloadManagerBase> spDMgt,
		         std::atomic<long long> *pLastRcvTic // in seconds since EPOCH
	             );
	~RDDispatcher();

	void Dispatch(std::shared_ptr<DDRFramework::BaseSocketContainer> spParentSocketContainer,
		          std::shared_ptr<DDRCommProto::CommonHeader> spHeader,
		          std::shared_ptr<google::protobuf::Message> spMsg) override;
private:
	std::shared_ptr<DownloadManagerBase> m_spDMgt;
	std::atomic<long long> *m_pLastTic;
};

}

#endif // __DDR_ROUTEUPLOADER_MSG_DISPATCHER_H_INCLUDED__
