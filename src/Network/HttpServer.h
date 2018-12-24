#ifndef HttpServer_h__
#define HttpServer_h__


#include "json/json.h"
#include "HttpServer/http_server.hpp"
#include <string>
#include <exception>
#include <iostream>


namespace DDRFramework
{

	class HttpServer
	{
	public:
		HttpServer();
		~HttpServer();


		const std::string currentDateTime();
		void Start(const std::string& address, const std::string& port,const  std::string& docroot);
		void Stop();

		void SaveJsonFile(Json::Value& value, std::string& name);
	protected:

		std::string m_RootPath;
		std::string m_Port;

	};
}
#endif // HttpServer_h__