/*!
 * File: BaseProcessor.h
 * Date: 2019/04/29 15:58
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Base Processor.Processor logic task while proto msg arrived
 *
*/
#ifndef BaseProcessor_h__
#define BaseProcessor_h__


#include "src/Utility/DDRMacro.h"
#include "src/Utility/Singleton.h"
#include "src/Network/TcpSocketContainer.h"

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
		std::map<std::string, std::shared_ptr<CommonHeader>> m_RouteMsgHeaderMap;

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