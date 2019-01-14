#ifndef BaseProcessor_h__
#define BaseProcessor_h__


#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "TcpSocketContainer.h"
#include "UdpSocketBase.h"
#include "BaseSocketContainer.h"
#include "../../proto/BaseCmd.pb.h"

using namespace DDRCommProto;
namespace DDRFramework
{
	class BaseSocketContainer;
	class BaseMessageDispatcher;

	class MsgRouterManager:public CSingleton<MsgRouterManager>
	{
	public:
		MsgRouterManager();
		~MsgRouterManager() {};

		std::shared_ptr<CommonHeader> FindCommonHeader(std::string bodytype);

		void SetCltType(eCltType type)
		{
			m_CltType = type;
		}

		CommonHeader_PassNode* RecordPassNode(std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<TcpSocketContainer> spSession);
		CommonHeader_PassNode* RecordPassNode(std::shared_ptr<DDRCommProto::CommonHeader> spHeader,std::shared_ptr<TcpSocketContainer> spSession,std::vector<int> dataptrs);
		bool ReturnPassNode(std::shared_ptr<DDRCommProto::CommonHeader> spHeader, CommonHeader_PassNode& passnode);
		bool IsLastPassNode(std::shared_ptr<DDRCommProto::CommonHeader> spHeader);
	private:
		std::map<string, std::shared_ptr<CommonHeader>> m_RouteMsgHeaderMap;

		eCltType m_CltType;
	};

	class BaseHeadRuleRouter
	{
	public:
		BaseHeadRuleRouter() {}

		virtual bool IgnoreBody(std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader,asio::streambuf& buf,int bodylen) {
			return false;
		};
	};


	class BaseProcessor
	{
	public:
		BaseProcessor(BaseMessageDispatcher& dispatcher):m_ParentDispatch(dispatcher) {}
		virtual void Process(std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg) {};
		virtual void AsyncProcess(std::shared_ptr<BaseSocketContainer> spSockContainer, std::shared_ptr<DDRCommProto::CommonHeader> spHeader, std::shared_ptr<google::protobuf::Message> spMsg) {};

		BaseMessageDispatcher& m_ParentDispatch;
	};
}


#endif // BaseProcessor_h__