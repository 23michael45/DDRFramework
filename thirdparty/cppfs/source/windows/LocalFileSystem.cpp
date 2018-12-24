
#include <cppfs/windows/LocalFileSystem.h>

#include <cppfs/FileHandle.h>
#include <cppfs/windows/LocalFileHandle.h>


namespace cppfs
{
	std::wstring StringToWString(const std::string& s)
	{
		std::wstring temp(s.length(), L' ');
		std::copy(s.begin(), s.end(), temp.begin());
		return temp;
	}


	std::string WStringToString(const std::wstring& s)
	{
		std::string temp(s.length(), ' ');
		std::copy(s.begin(), s.end(), temp.begin());
		return temp;
	}

#ifdef _WINDOWS
#include <string>
#include <windows.h>
#include "cppfs/windows/LocalFileSystem.h"

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

		std::string sformat = s;
		sformat = sformat.insert(pos, "(");

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
			sformat = sformat.insert(pos, ")(");
			pos += 3;


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
