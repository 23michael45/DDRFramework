#include <functional>
#include "../../Shared/src/Utility/Logger.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "HttpClient.h"
#include <curl/curl.h>
#include "../../Shared/thirdparty/curl/include/curl/easy.h"
#include <sstream>
#include <iostream>
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
		try
		{
			out.write(asio::buffer_cast<const char*>(buffer), asio::buffer_size(buffer));
			return curl_asio::data_action::success;
		}
		catch (std::exception& e)
		{
			DebugLog("%s", e.what());
		}
		return curl_asio::data_action::abort;
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

	size_t write_download_file(void *ptr, size_t size, size_t nmemb, void *pData) {

		string data((const char*)ptr, (size_t)size * nmemb);

		auto pSession = ((HttpSession*)pData);
		pSession->WriteFileStream(ptr, size * nmemb);

		return size * nmemb;
	}
	void HttpSession::GetThread(std::string url, std::string outfile)
	{
		m_pCurl = curl_easy_init();
	
		curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
		curl_easy_setopt(m_pCurl, CURLOPT_ACCEPT_ENCODING, "deflate");

		curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, write_download_file);

		m_OutFileStream.open(outfile, std::ios::binary);
		curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, this);
		/* Perform the request, res will get the return code */
		CURLcode res = curl_easy_perform(m_pCurl);
		/* Check for errors */
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
				curl_easy_strerror(res));
		}
		m_OutFileStream.close();

		curl_easy_cleanup(m_pCurl);
		DebugLog("HttpSession Entry Finish")

		if (m_OnDoneFunc)
		{
			m_OnDoneFunc(0);
		}
	}

	void HttpSession::DoGet(std::string& url, std::string outfile)
	{
		auto func = std::bind(&HttpSession::GetThread, shared_from_this(), url, outfile);
		std::thread t(func);
		t.detach();
	}










	size_t write_upload_file(void *buffer, size_t size, size_t nmemb, void *userp) {
		FILE *fptr = (FILE*)userp;
		fwrite(buffer, size, nmemb, fptr);

		return size * nmemb;
	}

	void HttpSession::DoPost(std::string url,std::string basedir, std::string inputfile)
	{
		auto func = std::bind(&HttpSession::PostThread, shared_from_this(), url, basedir,inputfile);
		std::thread t(func);
		t.detach();

	}
	void HttpSession::PostThread(std::string url, std::string basedir, std::string inputfile)
	{

		std::string fullpath = basedir + inputfile;
		std::ifstream in(fullpath.c_str(), std::ifstream::ate | std::ifstream::binary);
		in.seekg(0, std::ios::end);    // go to the end  
		int length = in.tellg();
		char* buffer = new char[length];
		in.read(buffer, length);       // read the whole file into the buffer  
		in.close();
	
		curl_global_init(CURL_GLOBAL_ALL);
		CURL* hCurl = curl_easy_init();
		if (hCurl != NULL)
		{
			//也许有Expect: 100-continue，去掉它
			curl_slist* pOptionList = NULL;
			pOptionList = curl_slist_append(pOptionList, "Expect:");
			curl_easy_setopt(hCurl, CURLOPT_HTTPHEADER, pOptionList);

			curl_httppost* pFormPost = NULL;
			curl_httppost* pLastElem = NULL;
			//上传文件，指定本地文件完整路径
			//curl_formadd(&pFormPost, &pLastElem, CURLFORM_COPYNAME, "file", CURLFORM_FILE, fullpath.c_str(), CURLFORM_CONTENTTYPE, "application/octet-stream", CURLFORM_END);

			//上传自定义文件内容的文件，CURLFORM_BUFFER指定服务端文件名
			//http://curl.haxx.se/libcurl/c/curl_formadd.html
			const char* file_info = inputfile.c_str();

			curl_formadd(&pFormPost, &pLastElem,
				CURLFORM_COPYNAME, "file",
				CURLFORM_BUFFER, inputfile.c_str(),
				CURLFORM_BUFFERPTR, buffer,
				CURLFORM_BUFFERLENGTH, length,
				CURLFORM_END);

			//不加一个结束的hfs服务端无法写入文件，一般不存在这种问题，这里加入只是为了测试.
			//curl_formadd(&pFormPost, &pLastElem, CURLFORM_COPYNAME, "end", CURLFORM_COPYCONTENTS, "end", CURLFORM_END);
			curl_easy_setopt(hCurl, CURLOPT_HTTPPOST, pFormPost);
			curl_easy_setopt(hCurl, CURLOPT_URL, url.c_str());

			CURLcode res = curl_easy_perform(hCurl);
			if (res != CURLE_OK)
			{
				std::wcout << "Error" << std::endl;
			}
			curl_formfree(pFormPost);
			curl_easy_cleanup(hCurl);
			delete[] buffer;
		}

		curl_global_cleanup();
	}

}