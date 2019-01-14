#include <functional>
#include "../../Shared/src/Utility/Logger.h"
#include "../../Shared/src/Utility/DDRMacro.h"
#include "HttpClient.h"
#include <curl/curl.h>
#include "../../Shared/thirdparty/curl/include/curl/easy.h"
#include <sstream>
#include <iostream>
#include "../../../Shared/thirdparty/cppfs/include/cppfs/windows/LocalFileSystem.h"
#include "../../../Shared/thirdparty/cppfs/include/cppfs/FilePath.h"
#include "../../../Shared/thirdparty/cppfs/include/cppfs/FileHandle.h"
#include "../../../Shared/thirdparty/cppfs/include/cppfs/fs.h"



namespace DDRFramework
{

	HttpSession::HttpSession():m_spWork(std::make_shared<asio::io_context::work>(m_ContextIO))
	{
	}

	HttpSession::~HttpSession()
	{
		DebugLog("HttpSession Destroy")
	}


	size_t write_download_file(void *ptr, size_t size, size_t nmemb, void *pData) {

		string data((const char*)ptr, (size_t)size * nmemb);

		auto pSession = ((HttpSession*)pData);
		pSession->WriteFileStream(ptr, size * nmemb);

		return size * nmemb;
	}
	void HttpSession::GetThread(std::vector<std::string> urls, std::vector<std::string> outfiles)
	{
		for (int i = 0 ; i< urls.size();i++)
		{
			GetOneFile(urls[i], outfiles[i]);
		}
		if (m_OnGetDoneFunc)
		{
			m_OnGetDoneFunc(1);
		}
	}

	void HttpSession::DoGet(std::string url, std::string outfile)
	{
		std::vector<std::string> urls;
		urls.push_back(url);
		std::vector<std::string> outfiles;
		outfiles.push_back(outfile);
		auto func = std::bind(&HttpSession::GetThread, shared_from_this(), urls, outfiles);
		std::thread t(func);
		t.detach();
	}


	void HttpSession::DoGet(std::vector<std::string> urls, std::vector<std::string> outfiles)
	{
		auto func = std::bind(&HttpSession::GetThread, shared_from_this(), urls, outfiles);
		std::thread t(func);
		t.detach();
	}

	void HttpSession::GetOneFile(std::string url, std::string outfile)
	{
		m_pCurl = curl_easy_init();

		curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());
		/* example.com is redirected, so we tell libcurl to follow redirection */
		curl_easy_setopt(m_pCurl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(m_pCurl, CURLOPT_NOSIGNAL, 1); //Prevent "longjmp causes uninitialized stack frame" bug
		curl_easy_setopt(m_pCurl, CURLOPT_ACCEPT_ENCODING, "deflate");

		curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, write_download_file);

		cppfs::FilePath fp(outfile);
		cppfs::FileHandle fhandel = cppfs::fs::open(fp.directoryPath());
		fhandel.createDirectory();

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

		
	}

	size_t write_upload_file(void *buffer, size_t size, size_t nmemb, void *userp) {
		FILE *fptr = (FILE*)userp;
		fwrite(buffer, size, nmemb, fptr);

		return size * nmemb;
	}

	void HttpSession::DoPost(std::string url,std::string basedir, std::string inputfile)
	{
		std::vector<std::string> vec;
		vec.push_back(inputfile);
		auto func = std::bind(&HttpSession::PostThread, shared_from_this(), url, basedir,vec);
		std::thread t(func);
		t.detach();

	}

	void HttpSession::DoPost(std::string url, std::string basedir, std::vector<std::string> inputfiles)
	{
		auto func = std::bind(&HttpSession::PostThread, shared_from_this(), url, basedir, inputfiles);
		std::thread t(func);
		t.detach();
	}

	void HttpSession::PostOneFile(std::string url, std::string basedir, std::string inputfile)
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

	void HttpSession::PostThread(std::string url, std::string basedir, std::vector<std::string> inputfiles)
	{
		for (auto inputfile : inputfiles)
		{
			PostOneFile(url, basedir,inputfile);
		}
		if (m_OnPostDoneFunc)
		{
			m_OnPostDoneFunc(1);
		}

	}

}