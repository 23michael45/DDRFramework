

#ifdef _WINDOWS
#include <cppfs/windows/LocalFileSystem.h>
#include <cppfs/windows/LocalFileHandle.h>
#endif
#include <cppfs/FileHandle.h>

namespace cppfs
{

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
