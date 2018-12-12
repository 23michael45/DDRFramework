#ifndef HttpServer_h__
#define HttpServer_h__


#include "json/json.h"
#include "HttpServer/http_server.hpp"
#include <string>
#include <exception>
#include <iostream>




class HttpServer
{
public:
	HttpServer();
	~HttpServer();


	const std::string currentDateTime();
	void Start(std::string& address, std::string& port, std::string& docroot);
	void Stop();

	void SaveJsonFile(Json::Value& value,std::string& name);
private:

	std::string mRootPath;

};

#endif // HttpServer_h__