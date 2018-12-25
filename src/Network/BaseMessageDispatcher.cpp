
#include "BaseMessageDispatcher.h"
#include "../Utility/DDRMacro.h"
#include "../Utility/Logger.h"


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
		auto btype = spHeader->bodytype();
		if (btype.find("HeartBeat") == std::string::npos)
		{
			spParentSocketContainer->PrintRemoteIP(btype);

		}
		if (m_ProcessorMap.find(spHeader->bodytype()) != m_ProcessorMap.end())
		{
			auto sp = m_ProcessorMap[spHeader->bodytype()];
			if (sp)
			{
				sp->Process(spParentSocketContainer, spHeader, spMsg);

				//spParentSocketContainer->GetIOContext().post(std::bind(&BaseProcessor::AsyncProcess, sp, spParentSocketContainer, spHeader, spMsg));

				auto spThread = std::make_shared<std::thread>(std::bind(&BaseMessageDispatcher::AsyncThreadEntry, shared_from_this(), sp, spParentSocketContainer, spHeader, spMsg));
				spThread->detach();
				//m_AsyncThreadSet.insert(spThread);
			
			}
			else
			{
				DebugLog("\nDispatch Error Processor Empty");
			}
		}
	}
	void BaseMessageDispatcher::AsyncThreadEntry(std::shared_ptr<BaseProcessor> spProcessor ,std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		if (spProcessor)
		{
			spProcessor->AsyncProcess(spSockContainer, spHeader, spMsg);
		}
	}

	void BaseMessageDispatcher::EraseThread(std::shared_ptr<std::thread> spThread)
	{
		//if (m_AsyncThreadSet.find(spThread) != m_AsyncThreadSet.end())
		//{
		//	
		//	//spThread->~thread();
		//	//m_AsyncThreadSet.erase(spThread);

		//}
	}
}