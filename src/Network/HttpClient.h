#ifndef HttpClient_h__
#define HttpClient_h__
#include <string>
#include <fstream>
#include "../../Shared/thirdparty/curl-asio/curl-asio.hpp"
#include "../../thirdparty/asio/include/asio.hpp"
namespace DDRFramework
{

	class  HttpSession
	{
	public:
		HttpSession();
		~HttpSession();

		curl_asio::data_action::type  on_transfer_data_read(std::ofstream &out, const asio::const_buffer& buffer);
		void on_transfer_done(curl_asio::transfer::ptr transfer, std::ofstream &out, const std::string &file, CURLcode result);
		void DoGet(std::string& url, std::string outfile);


	private:

	};

}
#endif // HttpClient_h__
