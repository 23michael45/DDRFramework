#ifndef TcpSocketContainer_h__
#define TcpSocketContainer_h__


#include "asio.hpp"
#include <map>
using asio::ip::tcp;

namespace DDRFramework
{
	class MessageSerializer;

	class TcpSocketContainer
	{
	public:
		TcpSocketContainer(asio::io_context &context) : m_Socket(context), m_bConnected(true)
		{

		}

		tcp::socket& GetSocket()
		{
			return m_Socket;
		}

		std::shared_ptr<MessageSerializer> GetSerializer()
		{
			return m_spSerializer;
		}
		void SetSerializer(std::shared_ptr<MessageSerializer> sp)
		{
			m_spSerializer = sp;
		}

	protected:

		std::shared_ptr<MessageSerializer> m_spSerializer;
		bool m_bConnected;
		tcp::socket m_Socket;
	};

}

#endif // TcpSocketContainer_h__
