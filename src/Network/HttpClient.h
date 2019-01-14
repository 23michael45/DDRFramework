#ifndef HttpClient_h__
#define HttpClient_h__
#include <string>
#include <fstream>
#include "../../Shared/thirdparty/curl/include/curl/curl.h"
#include "../../thirdparty/asio/include/asio.hpp"
#include <memory>
#include <string>
namespace DDRFramework
{
	size_t write_download_file(void *ptr, size_t size, size_t nmemb, void *pfile);
	size_t write_upload_file(void *buffer, size_t size, size_t nmemb, void *userp);
	class  HttpSession : public std::enable_shared_from_this<HttpSession>
	{
	public:
		HttpSession();
		~HttpSession();

		void DoGet(std::string url, std::string outfile);
		void DoGet(std::vector<std::string> urls, std::vector<std::string>  outfiles);
		
		void DoPost(std::string url, std::string basedir, std::string inputfile);
		void DoPost(std::string url, std::string basedir, std::vector<std::string> inputfiles); 

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
		void BindOnGetDoneFunc(std::function<void(float)> f)
		{
			m_OnGetDoneFunc = f;
		}
		void BindOnPostDoneFunc(std::function<void(float)> f)
		{
			m_OnPostDoneFunc = f;
		}
	private:
		void GetOneFile(std::string url, std::string outfile);
		void GetThread(std::vector<std::string> urls, std::vector<std::string> outfiles);
		void PostOneFile(std::string url, std::string basedir, std::string inputfile);
		void PostThread(std::string url, std::string basedir, std::vector<std::string> inputfiles);

		asio::io_context m_ContextIO;
		std::shared_ptr<asio::io_context::work> m_spWork;

		std::ofstream m_OutFileStream;
		CURL* m_pCurl;

		std::function<void(float)> m_OnReadDataFunc;
		std::function<void(float)> m_OnGetDoneFunc;
		std::function<void(float)> m_OnPostDoneFunc;
	};

}
#endif // HttpClient_h__
