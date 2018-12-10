#pragma once 
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "TcpSocketContainer.h"
#include "../../proto/BaseCmd.pb.h"


namespace DDRFramework
{
	class BaseHeadRuleRouter
	{
	public:
		BaseHeadRuleRouter() {}

		virtual bool IgnoreBody(std::shared_ptr<TcpSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader) {
			return false;
		};

	};


	class BaseProcessor
	{
	public:
		BaseProcessor() {}

		virtual void Process(std::shared_ptr<TcpSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg) = 0;
	};
}