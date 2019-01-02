#include "FileManagerBase.h"

#include <iostream>
#include <string>
#include <regex>
#include "../../thirdparty/cppfs/include/cppfs/fs.h"
#include "../../thirdparty/cppfs/include/cppfs/FileHandle.h"
#include "../../thirdparty/cppfs/include/cppfs/FileIterator.h"
#include "../../thirdparty/cppfs/include/cppfs/FilePath.h"

#ifdef _WINDOWS
#include "../../thirdparty/cppfs/include/cppfs/windows/LocalFileSystem.h"
#endif

#include "../../Shared/src/Utility/CommonFunc.h"
#include "../../../Shared/src/Utility/DDRMacro.h"
#include "../../../Shared/src/Utility/Logger.h"
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


	void FileManagerBase::CheckDir(std::string dir, std::vector<std::string>& vec)
	{
		cppfs::FileHandle fhandel = fs::open(dir);
		if (fhandel.exists())
		{
			if (fhandel.isDirectory())
			{
				auto files = fhandel.listFiles();




				for (auto file : files)
				{
					CheckDir(dir + "/" + file, vec);
				}
			}
			else
			{
				vec.push_back(dir);
			}

		}
	}
	std::vector<string> FileManagerBase::CheckFiles()
	{
		std::vector<std::string> files;
		CheckDir(m_RootPath, files);
		return files;

	}
}