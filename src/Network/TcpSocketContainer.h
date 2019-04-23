#ifndef TcpSocketContainer_h__
#define TcpSocketContainer_h__

#include "../../src/Utility/DDRMacro.h"
#include "asio.hpp"
#include <map>
#include "../Logic/BaseBehavior.h"

using asio::ip::tcp;

//Code Like This 
//std::shared_ptr<TcpClientSessionBase> spTcpClientSessionBase = std::make_shared<TcpClientSessionBase>(m_IOContext);
//std::shared_ptr<MessageSerializer> spMessageSerializer = std::make_shared<MessageSerializer>();
//auto spBaseMessageDispatcher = std::make_shared<BaseMessageDispatcher>();
//spTcpClientSessionBase->LoadSerializer(spMessageSerializer);
//spMessageSerializer->Init();
//spMessageSerializer->BindDispatcher(spBaseMessageDispatcher);



#define  BIND_IOCONTEXT_SERIALIZER_DISPATCHER(context,tcpinstace,serializer,dispatcher,headrulerouter)    std::shared_ptr<tcpinstace> sp##tcpinstace = std::make_shared<tcpinstace>(context);\
std::shared_ptr<serializer> sp##serializer = std::make_shared<serializer>();\
sp##tcpinstace->LoadSerializer(sp##serializer);\
auto sp##dispatcher = std::make_shared<dispatcher>();\
auto sp##headrulerouter = std::make_shared<headrulerouter>();\
sp##serializer->Init();\
sp##serializer->BindDispatcher(sp##dispatcher,sp##headrulerouter);

#define SHARED_FROM_BASE(CTYPE) std::shared_ptr<CTYPE> shared_from_base() {\
	return std::static_pointer_cast<CTYPE>(shared_from_this());\
}

namespace DDRFramework
{
	class MessageSerializer;
	class BaseProcessor;

	class TcpSocketContainer : public std::enable_shared_from_this<TcpSocketContainer>
	{
	public:
		TcpSocketContainer(asio::io_context &context);
		~TcpSocketContainer();


		void Start();

		virtual void CheckRead();
		virtual void CheckWrite();
		virtual void CheckBehavior();


		void HandleWrite(const asio::error_code& ec, size_t size);

		//send proto msg
		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
		//send proto msg with head
		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		//send bask to a client that route store in spheader
		void SendBack(std::shared_ptr<DDRCommProto::CommonHeader> spheader, std::shared_ptr<google::protobuf::Message> spmsg);
		//send proto msg buf with head
		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader, asio::streambuf& buf, int bodylen);
		//send raw buf
		void Send(std::shared_ptr<asio::streambuf> spbuf);
		void Send(const void* psrc, int len);

		tcp::socket& GetSocket();

		std::shared_ptr<MessageSerializer> GetSerializer();
		void LoadSerializer(std::shared_ptr<MessageSerializer> sp);

		void Release();



		virtual void OnStart() {};
		virtual void OnStop() {}

		void BindOnDisconnect(std::function<void(std::shared_ptr<TcpSocketContainer>)> f)
		{
			m_fOnSessionDisconnect = f;
		}
		void BindOnConnected(std::function<void(std::shared_ptr<TcpSocketContainer>)> f)
		{
			m_fOnSessionConnected = f;
		}	
		void BindOnConnectFailed(std::function<void(std::shared_ptr<TcpSocketContainer>)> f)
		{
			m_fOnSessionConnectFailed = f;
		}
		void BindOnConnectTimeout(std::function<void(std::shared_ptr<TcpSocketContainer>)> f)
		{
			m_fOnSessionConnectTimeout = f;
		}
		void BindOnHookReceive(std::function<void(asio::streambuf&)> f)
		{
			m_fOnHookReceiveData = f;
		}

		bool IsConnected()
		{
			return m_bConnected;
		}
		void Stop();
		void CallOnDisconnect();

		asio::io_context& GetIOContext()
		{
			return m_IOContext;
		}

		void BindBehavior(std::shared_ptr<BaseBehavior> behavior);
		std::shared_ptr<BaseBehavior> GetBehavior()
		{
			return m_spBehavior;
		}
		bool HasBehavior()
		{
			return m_spBehavior != nullptr;
		}

		std::string GetIPAddress();

		void SetRealtime(bool b);


		bool RegisterExternalProcessor(google::protobuf::Message& msg, std::shared_ptr<BaseProcessor> sp);
		bool UnregisterExternalProcessor(google::protobuf::Message& msg);


	protected:
		void PushData(asio::streambuf& buf);
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) {};
	


		asio::io_context& m_IOContext;
		std::shared_ptr<MessageSerializer> m_spSerializer;
		bool m_bConnected;
		tcp::socket m_Socket;


		asio::io_context::strand m_ReadWriteStrand;
		std::shared_ptr<asio::streambuf> m_CurrentWritingBuf;//use to protect buf release,cause consume crash


		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionDisconnect;
		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionConnected;
		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionConnectTimeout;
		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionConnectFailed;


		std::function<void(asio::streambuf&)> m_fOnHookReceiveData;


		std::shared_ptr<BaseBehavior> m_spBehavior;

		int m_iCheckWriteSleep;
		int m_iCheckReadSleep;


		std::string m_fromIP;
	private:
	};

}

#endif // TcpSocketContainer_h__
