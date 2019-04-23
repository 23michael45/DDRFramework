
#include "BaseMessageDispatcher.h"
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
		for (auto pair : m_ProcessorMap)
		{
			pair.second.reset();
		}
		DebugLog("BaseMessageDispatcher Destroy");
	}

	void BaseMessageDispatcher::Dispatch(std::shared_ptr<BaseSocketContainer> spParentSocketContainer, std::shared_ptr<CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		try
		{

			Hook(spParentSocketContainer, spHeader, spMsg);

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
					DebugLog("Dispatch Error Processor Empty");
				}
			}

		}
		catch (const std::exception& e)
		{

			DebugLog("Dispatch Exception %s", e.what())
		}
	}

	void BaseMessageDispatcher::Hook(std::shared_ptr<BaseSocketContainer> spParentSocketContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
	{
		auto btype = spHeader->bodytype();
		if (btype.find("HeartBeat") == std::string::npos && spParentSocketContainer->GetTcp() != nullptr)
		{
			spParentSocketContainer->PrintRemoteIP(btype);
		}
		else if (btype == "DDRCommProto.rspLogin")
		{
			rspLogin* pRaw = reinterpret_cast<rspLogin*>(spMsg.get());
			if (pRaw)
			{
				MsgRouterManager::Instance()->SetCltType(pRaw->yourrole());
			}
		}
		else if (btype == "DDRCommProto.rspRemoteLogin")
		{
			rspRemoteLogin* pRaw = reinterpret_cast<rspRemoteLogin*>(spMsg.get());
			if (pRaw)
			{

				MsgRouterManager::Instance()->SetCltType(pRaw->yourrole());
			}
		}
	}

	bool BaseMessageDispatcher::RegisterExternalProcessor(google::protobuf::Message& msg, std::shared_ptr<BaseProcessor> sp)
	{
		if (m_ProcessorMap.find(msg.GetTypeName()) != m_ProcessorMap.end())
		{
			return false;
		}
		else
		{
			m_ProcessorMap[msg.GetTypeName()] = sp;
			return true;
		}
	}

	bool BaseMessageDispatcher::UnregisterExternalProcessor(google::protobuf::Message& msg)
	{
		if (m_ProcessorMap.find(msg.GetTypeName()) != m_ProcessorMap.end())
		{
			m_ProcessorMap.erase(msg.GetTypeName());
			return true;
		}
		else
		{
			return false;
		}
	}

	void BaseMessageDispatcher::AsyncThreadEntry(std::shared_ptr<BaseProcessor> spProcessor, std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg)
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

	BaseUdpMessageDispatcher::BaseUdpMessageDispatcher()
	{

	}


	BaseUdpMessageDispatcher::~BaseUdpMessageDispatcher()
	{
		DebugLog("LocalClientUdpDispatcher Destroy");
	}

}