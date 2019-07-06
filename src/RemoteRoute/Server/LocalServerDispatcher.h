#ifndef LocalServerDispatcher_h__
#define LocalServerDispatcher_h__

#include "src/Network/BaseMessageDispatcher.h"
#include "src/Network/BaseSocketContainer.h"

#endif // LocalServerDispatcher_h__
class LocalServerDispatcher : public DDRFramework::BaseMessageDispatcher
{
public:
	LocalServerDispatcher();
	~LocalServerDispatcher();


	virtual void Dispatch(std::shared_ptr< DDRFramework::BaseSocketContainer> spParentSocketContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg) override;

};

