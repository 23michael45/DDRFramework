#include "HttpServer.h"
#include <fstream>
#include <exception>

namespace DDRFramework
{
	HttpServer::HttpServer()
	{
	}

	HttpServer::~HttpServer()
	{
	}
	// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
	const std::string HttpServer::currentDateTime() {
		time_t     now = time(0);
		struct tm  tstruct;
		char       buf[80];
		tstruct = *localtime(&now);
		strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

		return buf;
	}


	void HttpServer::Start(const std::string& address, const std::string& port,const std::string& docroot)
	{
		try
		{
			m_IPAddress = address;
			m_RootPath = docroot;
			m_Port = port;

			// Initialise the server.

			// Run the server until stopped.
			std::thread t(std::bind([](std::string& address, std::string& port, std::string& docroot) {

				http::server::server s(address, port, docroot);
				s.run();
			}, address, port, docroot));
			t.detach();

		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
	}
	void HttpServer::Stop()
	{

	}

	void HttpServer::SaveJsonFile(Json::Value& value, std::string& name)
	{
		try
		{
			std::ofstream f;
			f.open(m_RootPath + "/" + name + ".json", std::ofstream::out | std::ofstream::app);

			Json::StreamWriterBuilder wb;
			std::unique_ptr<Json::StreamWriter> writer(wb.newStreamWriter());
			writer->write(value, &f);
			f.close();
		}
		catch (std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}
}