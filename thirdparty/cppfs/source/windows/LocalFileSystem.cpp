
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
