
#include "BaseMessageDispatcher.h"
#include "../Utility/DDRMacro.h"


using namespace google::protobuf;
using namespace DDRCommProto;
namespace DDRFramework
{


	BaseMessageDispatcher::BaseMessageDispatcher()
	{


	}

	BaseMessageDispatcher::~BaseMessageDispatcher()
	{
		DebugLog("\nBaseMessageDispatcher Destroy");
	}

	void BaseMessageDispatcher::Dispatch(std::shared_ptr<BaseSocketContainer> spParentSocketContainer, std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (m_ProcessorMap.find(spHeader->bodytype()) != m_ProcessorMap.end())
		{
			auto sp = m_ProcessorMap[spHeader->bodytype()];
			if (sp)
			{
				sp->Process(spParentSocketContainer, spHeader, spMsg);

				//spParentSocketContainer->GetIOContext().post(std::bind(&BaseProcessor::AsyncProcess, sp, spParentSocketContainer, spHeader, spMsg));

				std::thread t(std::bind(&BaseProcessor::AsyncProcess, sp, spParentSocketContainer, spHeader, spMsg));
				t.detach();
			}
			else
			{
				DebugLog("\nDispatch Error Processor Empty");
			}
		}
	}
}