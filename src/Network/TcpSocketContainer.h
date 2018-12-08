#ifndef TcpSocketContainer_h__
#define TcpSocketContainer_h__

#include <google/protobuf/message.h>
#include "asio.hpp"
#include <map>
using asio::ip::tcp;

#define  BIND_IOCONTEXT_SERIALIZER_DISPATCHER(context,tcpinstace,serializer,dispatcher)    std::shared_ptr<tcpinstace> sp##tcpinstace = std::make_shared<tcpinstace>(context);\
std::shared_ptr<serializer> sp##serializer = std::make_shared<serializer>();\
sp##tcpinstace->LoadSerializer(sp##serializer);\
auto sp##dispatcher = std::make_shared<dispatcher>(sp##tcpinstace);\
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

		virtual void Update();

		void Send(google::protobuf::Message& msg);


		tcp::socket& GetSocket();

		std::shared_ptr<MessageSerializer> GetSerializer();
		void LoadSerializer(std::shared_ptr<MessageSerializer> sp);

		void UnloadSerializer();

		void BindOnDisconnect(std::function<void(std::string)> f)
		{
			m_fOnSessionDisconnect = f;
		}
		bool IsConnected()
		{
			return m_bConnected;
		}

	protected:
		virtual void StartWrite(asio::streambuf& buf) {};

		asio::io_context& m_IOContext;
		std::shared_ptr<MessageSerializer> m_spSerializer;
		bool m_bConnected;
		tcp::socket m_Socket;


		std::function<void(std::string)> m_fOnSessionDisconnect;

	private:
	};

}

#endif // TcpSocketContainer_h__
