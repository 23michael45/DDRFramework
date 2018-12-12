#pragma once
#include <map>
#include <memory.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "BaseProcessor.h"
#include "../../proto/BaseCmd.pb.h"
#include "BaseSocketContainer.h"

namespace DDRFramework
{
	class TcpSocketContainer;
	class BaseProcessor;
	class  BaseMessageDispatcher : public std::enable_shared_from_this<BaseMessageDispatcher>
	{
	public:
		BaseMessageDispatcher();
		~BaseMessageDispatcher();


		virtual void Dispatch(std::shared_ptr<BaseSocketContainer> spParentSocketContainer,std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);

		
		void AsyncThreadEntry(std::shared_ptr<BaseProcessor> spProcessor, std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);
		void EraseThread(std::shared_ptr<std::thread> spThread);
	protected:


		std::map<std::string, std::shared_ptr<BaseProcessor>> m_ProcessorMap;
		//std::set<std::shared_ptr<std::thread>> m_AsyncThreadSet;
	};

}