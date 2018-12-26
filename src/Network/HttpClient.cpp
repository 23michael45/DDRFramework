#include <functional>
#include "../../Shared/src/Utility/Logger.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "HttpClient.h"
namespace DDRFramework
{
	HttpSession::HttpSession()
	{
	}

	HttpSession::~HttpSession()
	{
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

			DebugLog("\nTransfer of %s  completed successfully ( %d  seconds)! Content saved to file: %s ", transfer->info().effective_url().c_str(), transfer->info().total_time(), file.c_str());
		}
		else
		{
			DebugLog("Transfer of %s  failed with error %i", transfer->info().effective_url().c_str(), result);
		}
	}


	void HttpSession::DoGet(std::string& url, std::string outfile)
	{
		asio::io_service io;
		curl_asio curl(io);
		curl_asio::transfer::ptr transfer = curl.create_transfer();
		if (transfer)
		{
			std::ofstream out(outfile);
			transfer->opt.protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
			transfer->opt.max_redirs = 5;
			transfer->opt.redir_protocols = CURLPROTO_HTTP | CURLPROTO_HTTPS;
			transfer->opt.follow_location = true;
			transfer->on_data_read = std::bind(&HttpSession::on_transfer_data_read,this, std::ref(out), std::placeholders::_1);
			transfer->on_done = std::bind(&HttpSession::on_transfer_done,this, transfer, std::ref(out), outfile, std::placeholders::_1);
			if (transfer->start(url))
			{
				io.run();
			}
		}
	}

}