#include <functional>
#include "../../Shared/src/Utility/Logger.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "HttpClient.h"
namespace DDRFramework
{
	HttpClientSession::HttpClientSession():m_spWork(std::make_shared<asio::io_context::work>(m_ContextIO))
	{
	}

	HttpClientSession::~HttpClientSession()
	{
		DebugLog("\nHttpSession Destroy")
	}



	curl_asio::data_action::type HttpClientSession::on_transfer_data_read(std::ofstream &out, const asio::const_buffer& buffer)
	{
		out.write(asio::buffer_cast<const char*>(buffer), asio::buffer_size(buffer));
		return curl_asio::data_action::success;
	}

	void HttpClientSession::on_transfer_done(curl_asio::transfer::ptr transfer, std::ofstream &out, const std::string &file, CURLcode result)
	{
		if (result == CURLE_OK)
		{
			out.close();

			DebugLog("\nTransfer of %s  completed successfully ( %d  seconds)! Content saved to file: %s ", transfer->info().effective_url().c_str(), transfer->info().total_time(), file.c_str());
		}
		else
		{
			DebugLog("Transfer of %s  failed with error %i", transfer->info().effective_url().c_str(), result);
		}
		m_spWork.reset();
	}
	void HttpClientSession::ThreadEntry(std::string& url, std::string outfile)
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
			transfer->on_data_read = std::bind(&HttpClientSession::on_transfer_data_read, shared_from_this(), std::ref(out), std::placeholders::_1);
			transfer->on_done = std::bind(&HttpClientSession::on_transfer_done, shared_from_this(), transfer, std::ref(out), outfile, std::placeholders::_1);
			if (transfer->start(url))
			{
				m_ContextIO.run();
			}
		}
	}

	void HttpClientSession::DoGet(std::string& url, std::string outfile)
	{
		auto func = std::bind(&HttpClientSession::ThreadEntry, shared_from_this(), url, outfile);
		auto spT = std::make_shared<std::thread>(func);
		spT->join();
		
		//ThreadEntry(url,outfile);
	}
	void HttpClientSession::DoPost()
	{
		DebugLog("\nPost");
	}

}