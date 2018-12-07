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
	class  BaseMessageDispatcher
	{
	public:
		BaseMessageDispatcher(std::shared_ptr<TcpSocketContainer> sp);
		~BaseMessageDispatcher();


		virtual void Dispatch(std::shared_ptr<TcpSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg);

	protected:

		std::map<std::string, std::shared_ptr<BaseProcessor>> m_ProcessorMap;
		std::shared_ptr<TcpSocketContainer> m_spParentSocketContainer;

	};

}