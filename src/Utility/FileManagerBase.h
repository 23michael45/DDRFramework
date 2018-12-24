#ifndef FileManagerBase_h__
#define FileManagerBase_h__



#include <filesystem>
#include <vector>
#include <string>
#include "Singleton.h"

namespace DDRFramework
{

	class FileManagerBase
	{
	public:
		FileManagerBase();
		~FileManagerBase();

		void SetRootPath(std::string root);
		std::vector<std::string> CheckFiles();
	private:

		void CheckDir(std::string dir, std::vector<std::string>& vec);

		std::string m_RootPath;
	};
}
#endif // FileManagerBase_h__
