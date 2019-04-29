/*!
 * File: HttpServer.h
 * Date: 2019/04/29 15:56
 *
 * Author: michael
 * Contact: michael2345@live.cn
 *
 * Description:Http Server
 *
*/
#ifndef HttpServer_h__
#define HttpServer_h__


#include "json/json.h"
#include "HttpServer/http_server.hpp"
#include <string>
#include <exception>
#include <iostream>
#include <memory>

#include "HttpServer/connection.hpp"
#include "HttpServer/connection_manager.hpp"
#include "HttpServer/request_handler.hpp"

using namespace http::server;
namespace DDRFramework
{

	class HttpServer
	{
	public:
		HttpServer();
		~HttpServer();

		void Init();

		const std::string currentDateTime();
		void Start(const std::string& address, const std::string& port,const  std::string& docroot);

		void Stop();

		void Entry();

		void SaveJsonFile(Json::Value& value, std::string& name);
	protected:


		void DelayStart();
		void DelayStop();

		std::string m_RootPath;
		std::string m_Port;
		std::string m_IPAddress;
		

		asio::io_context m_IOContext;
		asio::io_context::strand m_Strand;
		std::shared_ptr< asio::io_service::work > m_spWork;

		bool m_bWorking;

		/// Perform an asynchronous accept operation.
		void do_accept();//异步接收新连接

		/// Wait for a request to stop the server.
		void do_await_stop();//异步等待信号


		/// The signal_set is used to register for process termination notifications.
		asio::signal_set m_Signals;//注册中止消息

		/// Acceptor used to listen for incoming connections.
		asio::ip::tcp::acceptor m_Acceptor;//监听套接字

		/// The connection manager which owns all live connections.
		connection_manager m_ConnectionManager;

		/// The next socket to be accepted.
		asio::ip::tcp::socket m_Socket;//已连接套接字

		/// The handler for all incoming requests.
		std::shared_ptr<request_handler> m_spRequestHandler;
	};
}
#endif // HttpServer_h__