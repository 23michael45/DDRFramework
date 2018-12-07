
#include "BaseMessageDispatcher.h"



using namespace google::protobuf;
using namespace DDRCommProto;
namespace DDRFramework
{


	BaseMessageDispatcher::BaseMessageDispatcher(std::shared_ptr<TcpSocketContainer> sp) : m_spParentSocketContainer(sp)
	{


	}

	BaseMessageDispatcher::~BaseMessageDispatcher()
	{
	}

	void BaseMessageDispatcher::Dispatch(std::shared_ptr<TcpSocketContainer> spSockContainer, std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (m_ProcessorMap.find(spHeader->bodytype()) != m_ProcessorMap.end())
		{
			m_ProcessorMap[spHeader->bodytype()]->Process(spSockContainer, spHeader,spMsg);
		}
	}
}