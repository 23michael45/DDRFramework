
#pragma once


#include <memory>
#include <string>
#include <cppfs/AbstractFileSystem.h>


namespace cppfs
{


	/**
	*  @brief
	*    Representation of the local file system
	*/
	class CPPFS_API LocalFileSystem : public AbstractFileSystem, public std::enable_shared_from_this<LocalFileSystem>
	{
	public:
		/**
		*  @brief
		*    Constructor
		*/
		LocalFileSystem();

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~LocalFileSystem();

		// Virtual AbstractFileSystem functions
		virtual FileHandle open(const std::string & path) override;
		virtual FileHandle open(std::string && path) override;
	};

	std::wstring StringToWString(const std::string& s);
	std::string WStringToString(const std::wstring& s);
	std::string getexepath();
	std::string getStartWildRegex(const std::string& s);


} // namespace cppfs
