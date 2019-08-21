#include "FileManagerBase.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <regex>
#include <mutex>
#include "../../thirdparty/cppfs/include/cppfs/fs.h"
#include "../../thirdparty/cppfs/include/cppfs/FileHandle.h"
#include "../../thirdparty/cppfs/include/cppfs/FileIterator.h"
#include "../../thirdparty/cppfs/include/cppfs/FilePath.h"

#ifdef _WINDOWS
#include "../../thirdparty/cppfs/include/cppfs/windows/LocalFileSystem.h"
#endif

#include "src/Utility/CommonFunc.h"
#include "src/Utility/LoggerDef.h"
using namespace cppfs;

namespace DDRFramework
{

	FileManagerBase::FileManagerBase()
	{

		cppfs::FilePath path(DDRFramework::getexepath());
		m_RootPath = path.directoryPath();


	}
	FileManagerBase::~FileManagerBase()
	{

	}

	void FileManagerBase::SetRootPath(std::string root)
	{
		m_RootPath = root;
	}


	std::string FileManagerBase::GetRootPath()
	{
		return m_RootPath;
	}


	std::vector<std::string> FileManagerBase::MatchDir(std::string dir, std::string fmt)
	{
		std::lock_guard<std::mutex> lock(m_FileMutex);
		std::string finalfmt = DDRFramework::getStarWildRegex(fmt, false);


		std::vector<std::string> vec;
		std::string full = m_RootPath + "/" + dir;
		cppfs::FileHandle fhandel = fs::open(full);
		if (fhandel.exists())
		{
			if (fhandel.isDirectory())
			{
				auto files = fhandel.listFiles();

				
				for (auto subfile : files)
				{
					if (std::regex_match(subfile, std::regex(finalfmt)))
					{
						auto value = dir + "/" + subfile;
						value = DDRFramework::MBToUTF8String(value);
						vec.push_back(value);
					}


				}
			}

		}
		return vec;
	}


	void FileManagerBase::SetIgnoreMatchDir(std::set<string> ignoreList)
	{
		m_IgnoreList = ignoreList;
	}

	std::vector<std::string> FileManagerBase::Match(std::string fmt,std::string root)
	{
		std::string fullroot = m_RootPath + root;
		fullroot = replace_all(fullroot, "///", "/");
		fullroot = replace_all(fullroot, "//", "/");
		std::string finalfmt = DDRFramework::getStarWildRegex(fmt,true, fullroot);

		std::string full = m_RootPath + "/" + root;
		full = replace_all(full, "///", "/");
		full = replace_all(full, "//", "/");

		std::vector<string> vec;
		MatchNode(full, finalfmt, 0, vec);
		return vec;
	}

	void FileManagerBase::MatchNode(std::string dir, std::string finalfmt, int level, std::vector<string> &vec)
	{

		cppfs::FileHandle fhandel = fs::open(dir);
		if (fhandel.exists())
		{



			if (fhandel.isDirectory())
			{
				
				auto files = fhandel.listFiles();
				for (auto file : files)
				{
					if (m_IgnoreList.find(file) != m_IgnoreList.end())
					{
						continue;
					}

					MatchNode(dir + "/" + file, finalfmt, level + 1, vec);
				}
			}
			else
			{
				if (std::regex_match(dir, std::regex(finalfmt)))
				{
					std::string value = DDRFramework::MBToUTF8String(dir);
					vec.push_back(value);
				}
			}
		}

	}




	std::string FileManagerBase::HttpAddr2BaseDir(std::string httpaddr)
	{
		int index = httpaddr.find_first_of(':');
		index = httpaddr.find(':', index + 1);
		index = httpaddr.find('/', index + 1);

		std::string full = httpaddr.replace(httpaddr.begin(), httpaddr.begin() + index, m_RootPath);
		full = replace_all(full, "///", "/");
		full = replace_all(full, "//", "/");

		return full;
	}

	std::string FileManagerBase::GetRelativeDirFromHttp(std::string httpaddr)
	{
		int index = httpaddr.find_first_of(':');
		index = httpaddr.find(':', index + 1);
		index = httpaddr.find('/', index + 1);

		std::string relativepath = httpaddr.replace(httpaddr.begin(), httpaddr.begin() + index, "");
		relativepath = replace_all(relativepath, "///", "/");
		relativepath = replace_all(relativepath, "//", "/");

		return relativepath;

	}	
	std::string FileManagerBase::GetRelativeDirFromFull(std::string fullpath)
	{
		std::string relativepath = fullpath.replace(fullpath.begin(), fullpath.begin() + m_RootPath.length(), "");
		relativepath = replace_all(relativepath, "///", "/");
		relativepath = replace_all(relativepath, "//", "/");

		return relativepath;

	}
	std::string FileManagerBase::GetFullDirFromRelative(std::string relativepath)
	{
		std::string full = m_RootPath + relativepath;	
		full = replace_all(relativepath, "//", "/");
		return full;
	}

	bool FileManagerBase::FileExist(std::string url)
	{
		std::lock_guard<std::mutex> lock(m_FileMutex);
		std::string full = url;
		if (url.substr(0, 4) == "http")
		{
			full = HttpAddr2BaseDir(url);
		}

		cppfs::FileHandle fhandel = fs::open(full);
		if (fhandel.exists())
		{
			return true;
		}
		return false;
	}


}