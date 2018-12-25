#ifndef TcpSocketContainer_h__
#define TcpSocketContainer_h__

#include <google/protobuf/message.h>
#include "../../proto/BaseCmd.pb.h"
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


namespace DDRFramework
{
	class MessageSerializer;

	class TcpSocketContainer : public std::enable_shared_from_this<TcpSocketContainer>
	{
	public:
		TcpSocketContainer(asio::io_context &context);
		~TcpSocketContainer();


		virtual void Start();

		virtual void CheckRead();
		virtual void CheckWrite();
		virtual void CheckBehavior();


		void HandleWrite(const asio::error_code& ec, size_t size);

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);
		void Send(std::shared_ptr<DDRCommProto::CommonHeader> spheader,std::shared_ptr<google::protobuf::Message> spmsg);
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

	protected:
		void PushData(asio::streambuf& buf);
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) {};
	


		asio::io_context& m_IOContext;
		std::shared_ptr<MessageSerializer> m_spSerializer;
		bool m_bConnected;
		tcp::socket m_Socket;


		asio::io_context::strand m_ReadWriteStrand;


		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionDisconnect;
		std::function<void(std::shared_ptr<TcpSocketContainer>)> m_fOnSessionConnected;


		std::function<void(asio::streambuf&)> m_fOnHookReceiveData;


		std::shared_ptr<BaseBehavior> m_spBehavior;

		int m_iCheckWriteSleep;
		int m_iCheckReadSleep;
	private:
	};

}

#endif // TcpSocketContainer_h__
