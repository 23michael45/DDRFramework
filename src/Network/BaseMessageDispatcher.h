#pragma once
#include <map>
#include <memory.h>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "BaseProcessor.h"
#include "../../proto/BaseCmd.pb.h"

namespace DDRFramework
{
	class TcpSocketContainer;
	class  BaseMessageDispatcher
	{
	public:
		BaseMessageDispatcher();
		~BaseMessageDispatcher();


		virtual void Dispatch(std::shared_ptr<TcpSocketContainer> spParentSocketContainer,std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);

	protected:

		std::map<std::string, std::shared_ptr<BaseProcessor>> m_ProcessorMap;

	};

}