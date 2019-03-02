#ifndef WebSocketClient_h__
#define WebSocketClient_h__

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>



namespace DDRFramework
{
	typedef websocketpp::client<websocketpp::config::asio_client> client;

	class websocket_endpoint;

	class connection_metadata {
	public:
		typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

		connection_metadata(int id, websocketpp::connection_hdl hdl, std::string uri,websocket_endpoint* pendpoint);

		void on_open(client * c, websocketpp::connection_hdl hdl);

		void on_fail(client * c, websocketpp::connection_hdl hdl);

		void on_close(client * c, websocketpp::connection_hdl hdl);

		void on_message(websocketpp::connection_hdl, client::message_ptr msg);

		websocketpp::connection_hdl get_hdl() const {
			return m_hdl;
		}

		int get_id() const;

		std::string get_status() const;


		friend std::ostream & operator<< (std::ostream & out, connection_metadata const & data);
	private:
		int m_id;
		websocketpp::connection_hdl m_hdl;
		std::string m_status;
		std::string m_uri;
		std::string m_server;
		std::string m_error_reason;
		websocket_endpoint* m_pendpoint;
	};


	class websocket_endpoint {
	public:
		websocket_endpoint();

		~websocket_endpoint();

		int connect(std::string const & uri);

		void close(int id, websocketpp::close::status::value code, std::string reason);

		void send(int id, std::string message);

		connection_metadata::ptr get_metadata(int id) const;

		void onmessage(std::string msg);

		void bindonmessage(std::function<void(std::string)> func);
	private:
		typedef std::map<int, connection_metadata::ptr> con_list;

		client m_endpoint;
		websocketpp::lib::shared_ptr<websocketpp::lib::thread> m_thread;

		con_list m_connection_list;
		int m_next_id;

		std::function<void(std::string)> m_onmsg;
	};


}
#endif // WebSocketClient_h__
