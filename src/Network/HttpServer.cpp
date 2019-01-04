#include "HttpServer.h"
#include <fstream>
#include <exception>

namespace DDRFramework
{
	HttpServer::HttpServer() : m_Strand(m_IOContext),
		m_spWork(std::make_shared< asio::io_context::work>(m_IOContext)),
		m_Signals(m_IOContext),
		m_Acceptor(m_IOContext),
		m_ConnectionManager(),
		m_Socket(m_IOContext),
		m_bWorking(false)
	{
	}

	HttpServer::~HttpServer()
	{
		m_spWork.reset();
	}

	void HttpServer::Init()
	{
		std::thread t(std::bind(&HttpServer::Entry, this));
		t.detach();
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


	void HttpServer::Start(const std::string& address, const std::string& port, const std::string& docroot)
	{
		if (m_bWorking)
		{
			return;
		}

		try
		{
			m_bWorking = true;
			m_IPAddress = address;
			m_RootPath = docroot;
			m_Port = port;

			m_Strand.post(std::bind(&HttpServer::DelayStart, this));

		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
	}
	void HttpServer::Stop()
	{
		m_bWorking = false;
		m_Strand.post(std::bind(&HttpServer::DelayStop, this));
	}

	void HttpServer::Entry()
	{
		m_IOContext.run();
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

	void HttpServer::DelayStart()
	{
		try
		{

			// Register to handle the signals that indicate when the server should exit.
			// It is safe to register for the same signal multiple times in a program,
			// provided all registration for the specified signal is made through Asio.
			m_Signals.add(SIGINT);
			m_Signals.add(SIGTERM);

			do_await_stop();

			// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
			asio::ip::tcp::resolver resolver(m_IOContext);
			asio::ip::tcp::endpoint endpoint = *resolver.resolve({ m_IPAddress, m_Port });
			m_Acceptor.open(endpoint.protocol());
			m_Acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
			m_Acceptor.bind(endpoint);
			m_Acceptor.listen();

			do_accept();

			if (m_spRequestHandler)
			{
				m_spRequestHandler.reset();
			}
			m_spRequestHandler = std::make_shared<request_handler>(m_RootPath);

		}
		catch (std::exception& e)
		{
			std::cerr << "exception: " << e.what() << "\n";
		}
	}

	void HttpServer::DelayStop()
	{
		m_Acceptor.close();
		m_ConnectionManager.stop_all();
	}


	void HttpServer::do_accept()//异步接收连接
	{
		if (m_bWorking)
		{

			m_Acceptor.async_accept(m_Socket,
				[this](asio::error_code ec)
			{
				// Check whether the server was stopped by a signal before this
				// completion handler had a chance to run.
				if (!m_Acceptor.is_open())
				{
					return;
				}

				if (!ec)
				{
					m_ConnectionManager.start(std::make_shared<connection>(
						std::move(m_Socket), m_ConnectionManager, *m_spRequestHandler.get()));
				}

				do_accept();
			});
		}
	}

	void HttpServer::do_await_stop()
	{
		m_Signals.async_wait(
			[this](asio::error_code /*ec*/, int /*signo*/)
		{
			// The server is stopped by cancelling all outstanding asynchronous
			// operations. Once all operations have finished the io_service::run()
			// call will exit.
			m_Acceptor.close();
			m_ConnectionManager.stop_all();
		});
	}
}