#include <cppfs/windows/LocalFileSystem.h>
#include <cppfs/windows/LocalFileHandle.h>
#include <cppfs/FileHandle.h>

#if defined(WIN32)
#include <windows.h>
#endif

namespace cppfs
{
	//std::wstring s2w(const std::string& utf8string)
	//{
	//	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//	return conv.from_bytes(utf8string);

	//}


	//std::string w2s(const std::wstring& utf16Str)
	//{
	//	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	//	return conv.to_bytes(utf16Str);
	//}



	std::wstring StringToWString(const std::string& str)
	{
#if defined(WIN32)
		size_t sz = str.length();
		int nd = MultiByteToWideChar(CP_ACP, 0, &str[0], sz, NULL, 0);
		std::wstring ret(nd, 0);
		int w = MultiByteToWideChar(CP_ACP, 0, &str[0], sz, &ret[0], nd);
		if (str.length() != sz) {

			throw std::exception("StringToWString Err");
		}
		return ret;
#else
		const char* p = str.c_str();
		size_t len = str.length();
		size_t sz = len * sizeof(wchar_t);
		wchar_t* tp = new wchar_t[sz];
		size_t w = mbstowcs(tp, p, sz);
		if (w != len) {
			delete[] tp;
			throw std::exception("StringToWString Err");
		}
		std::wstring ret(tp);
		delete[] tp;
		return ret;
#endif
		
	}


	std::string WStringToString(const std::wstring& str)
	{
		size_t sz = str.length();
#if defined(WIN32)
		int nd = WideCharToMultiByte(CP_ACP, 0, &str[0], sz, NULL, 0, NULL, NULL);
		std::string ret(nd, 0);
		int w = WideCharToMultiByte(CP_ACP, 0, &str[0], sz, &ret[0], nd, NULL, NULL);
		/*if (ret.length() != sz) {
			throw std::exception("WStringToString Err");
		}*/
		return ret;
#else
		const wchar_t* p = str.c_str();
		char* tp = new char[sz];
		size_t w = wcstombs(tp, p, sz);
		if (w != sz) {
			delete[] tp;

			throw std::exception("WStringToString Err");
		}
		std::string ret(tp);
		delete[] tp;
		return ret;
#endif
	}

#ifdef _WINDOWS

	std::string getexepath()
	{
		wchar_t result[MAX_PATH];

		std::wstring wstr = std::wstring(result, GetModuleFileName(NULL, result, MAX_PATH));
		return WStringToString(wstr);
	}
#else 
#include <string>
#include <limits.h>
#include <unistd.h>

	std::string getexepath()
	{
		char result[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
		return std::string(result, (count > 0) ? count : 0);
	}


#endif

	std::string getStartWildRegex(const std::string& s)
	{
		size_t pos = 0;
		std::vector<size_t> posvec;

		std::string sformat = "(.*)(" + s;
		pos = 4;
		do
		{

			pos = sformat.find('*', pos);
			if (pos == std::string::npos)
			{
				sformat = sformat.insert(sformat.length(), ")");
				break;
			}

			sformat = sformat.insert(pos, ")(.");
			pos += 4;

			if (pos == sformat.length())
			{
				sformat = sformat.insert(pos, ")");
				break;

			}
			else if (pos < sformat.length())
			{

				sformat = sformat.insert(pos, ")(");
				pos += 3;
			}


		} while (pos > 0);
		return sformat;
	}

	LocalFileSystem::LocalFileSystem()
	{
	}

	LocalFileSystem::~LocalFileSystem()
	{
	}

	FileHandle LocalFileSystem::open(const std::string & path)
	{
		return open(std::string(path));
	}

	FileHandle LocalFileSystem::open(std::string && path)
	{
		return FileHandle(
			std::unique_ptr<AbstractFileHandleBackend>(
				new LocalFileHandle(shared_from_this(), path)
				)
		);
	}


} // namespace cppfs
