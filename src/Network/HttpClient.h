#ifndef HttpClient_h__
#define HttpClient_h__
#include <string>
#include <fstream>
#include "../../Shared/thirdparty/curl-asio/curl-asio.hpp"
#include "../../thirdparty/asio/include/asio.hpp"
#include <memory>
#include <string>
namespace DDRFramework
{
	size_t write_file(void *ptr, size_t size, size_t nmemb, void *pfile);
	class  HttpSession : public std::enable_shared_from_this<HttpSession>
	{
	public:
		HttpSession();
		~HttpSession();

		curl_asio::data_action::type  on_transfer_data_read(std::ofstream &out, const asio::const_buffer& buffer);
		void on_transfer_done(curl_asio::transfer::ptr transfer, std::ofstream &out, const std::string &file, CURLcode result);
	

		void DoGet(std::string& url, std::string outfile);

		void ThreadEntry(std::string url, std::string outfile);
		void DoPost();

		void WriteFileStream(const void* ptr,int len)
		{
			m_OutFileStream.write((const char*)ptr, len);
			if (m_OnReadDataFunc)
			{
				m_OnReadDataFunc(len);
			}
		}
		void BindOnReadDataFunc(std::function<void(float)> f)
		{
			m_OnReadDataFunc = f;
		}
		void BindOnDoneFunc(std::function<void(float)> f)
		{
			m_OnDoneFunc = f;
		}
	private:

		asio::io_context m_ContextIO;
		std::shared_ptr<asio::io_context::work> m_spWork;

		std::ofstream m_OutFileStream;
		CURL* m_pCurl;

		std::function<void(float)> m_OnReadDataFunc;
		std::function<void(float)> m_OnDoneFunc;
	};

}
#endif // HttpClient_h__
