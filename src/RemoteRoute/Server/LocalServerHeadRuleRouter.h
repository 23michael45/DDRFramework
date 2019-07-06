#ifndef LocalServerHeadRuleRouter_h__
#define LocalServerHeadRuleRouter_h__
#include "src/Network/BaseProcessor.h"
#include "asio.hpp"
using namespace DDRFramework;
class LocalServerHeadRuleRouter : public BaseHeadRuleRouter
{
public:
	LocalServerHeadRuleRouter();
	~LocalServerHeadRuleRouter();

	virtual bool IgnoreBody(std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, asio::streambuf& buf, int bodylen) override;
};


#endif // LocalServerHeadRuleRouter_h__