
#include "BaseMessageDispatcher.h"
#include "../Utility/DDRMacro.h"


using namespace google::protobuf;
using namespace DDRCommProto;
namespace DDRFramework
{


	BaseMessageDispatcher::BaseMessageDispatcher(std::shared_ptr<TcpSocketContainer> sp) : m_spParentSocketContainer(sp)
	{


	}

	BaseMessageDispatcher::~BaseMessageDispatcher()
	{
		DebugLog("\nBaseMessageDispatcher Destroy");
	}

	void BaseMessageDispatcher::Dispatch(std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (m_ProcessorMap.find(spHeader->bodytype()) != m_ProcessorMap.end())
		{
			auto sp = m_ProcessorMap[spHeader->bodytype()];
			if (sp)
			{
				sp->Process(m_spParentSocketContainer, spHeader, spMsg);

			}
			else
			{
				DebugLog("\nDispatch Error Processor Empty");
			}
		}
	}
}