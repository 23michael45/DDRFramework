#ifndef TcpSocketContainer_h__
#define TcpSocketContainer_h__

#include <google/protobuf/message.h>
#include "asio.hpp"
#include <map>
using asio::ip::tcp;

//Code Like This 
//std::shared_ptr<TcpClientSessionBase> spTcpClientSessionBase = std::make_shared<TcpClientSessionBase>(m_IOContext);
//std::shared_ptr<MessageSerializer> spMessageSerializer = std::make_shared<MessageSerializer>();
//auto spBaseMessageDispatcher = std::make_shared<BaseMessageDispatcher>();
//spTcpClientSessionBase->LoadSerializer(spMessageSerializer);
//spMessageSerializer->Init();
//spMessageSerializer->BindDispatcher(spBaseMessageDispatcher);

#define  BIND_IOCONTEXT_SERIALIZER_DISPATCHER(context,tcpinstace,serializer,dispatcher)    std::shared_ptr<tcpinstace> sp##tcpinstace = std::make_shared<tcpinstace>(context);\
std::shared_ptr<serializer> sp##serializer = std::make_shared<serializer>();\
sp##tcpinstace->LoadSerializer(sp##serializer);\
auto sp##dispatcher = std::make_shared<dispatcher>();\
sp##serializer->Init();\
sp##serializer->BindDispatcher(sp##dispatcher);


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

		void Send(std::shared_ptr<google::protobuf::Message> spmsg);


		tcp::socket& GetSocket();

		std::shared_ptr<MessageSerializer> GetSerializer();
		void LoadSerializer(std::shared_ptr<MessageSerializer> sp);

		void UnloadSerializer();

		void BindOnDisconnect(std::function<void(TcpSocketContainer&)> f)
		{
			m_fOnSessionDisconnect = f;
		}
		bool IsConnected()
		{
			return m_bConnected;
		}
		void CloseSocket();
		void CallOnDisconnect();
	protected:
		void PushData(asio::streambuf& buf);
		virtual void StartWrite(std::shared_ptr<asio::streambuf> spbuf) {};
	


		asio::io_context& m_IOContext;
		std::shared_ptr<MessageSerializer> m_spSerializer;
		bool m_bConnected;
		tcp::socket m_Socket;


		asio::io_context::strand m_ReadStrand;
		asio::io_context::strand m_WriteStrand;


		std::function<void(TcpSocketContainer&)> m_fOnSessionDisconnect;

	private:
	};

}

#endif // TcpSocketContainer_h__
