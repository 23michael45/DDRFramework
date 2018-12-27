#include <functional>
#include "../../Shared/src/Utility/Logger.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "HttpClient.h"
namespace DDRFramework
{
	HttpSession::HttpSession():m_spWork(std::make_shared<asio::io_context::work>(m_ContextIO))
	{
	}

	HttpSession::~HttpSession()
	{
		DebugLog("HttpSession Destroy")
	}



	curl_asio::data_action::type HttpSession::on_transfer_data_read(std::ofstream &out, const asio::const_buffer& buffer)
	{
		out.write(asio::buffer_cast<const char*>(buffer), asio::buffer_size(buffer));
		return curl_asio::data_action::success;
	}

	void HttpSession::on_transfer_done(curl_asio::transfer::ptr transfer, std::ofstream &out, const std::string &file, CURLcode result)
	{
		if (result == CURLE_OK)
		{
			out.close();

			DebugLog("Transfer of %s  completed successfully ( %d  seconds)! Content saved to file: %s ", transfer->info().effective_url().c_str(), transfer->info().total_time(), file.c_str());
		}
		else
		{
			DebugLog("Transfer of %s  failed with error %i", transfer->info().effective_url().c_str(), result);
		}
		transfer->on_data_read = nullptr;
		transfer->on_done = nullptr;
		transfer.reset();
		m_spWork.reset();
	}
	void HttpSession::ThreadEntry(std::string url, std::string outfile)
	{
		curl_asio curl(m_ContextIO);
		curl_asio::transfer::ptr transfer = curl.create_transfer();
		if (transfer)
		{
			std::ofstream out(outfile);
			transfer->opt.protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
			transfer->opt.max_redirs = 5;
			transfer->opt.redir_protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
			transfer->opt.follow_location = true;
			transfer->on_data_read = std::bind(&HttpSession::on_transfer_data_read, shared_from_this(), std::ref(out), std::placeholders::_1);
			transfer->on_done = std::bind(&HttpSession::on_transfer_done, shared_from_this(), transfer, std::ref(out), outfile, std::placeholders::_1);
			if (transfer->start(url))
			{
				m_ContextIO.run();
			}
		}


		DebugLog("HttpSession Entry Finish")
	}

	void HttpSession::DoGet(std::string& url, std::string outfile)
	{
		auto func = std::bind(&HttpSession::ThreadEntry, shared_from_this(), url, outfile);
		std::thread t(func);
		t.detach();
	}
	void HttpSession::DoPost()
	{
		DebugLog("Post");
	}

}