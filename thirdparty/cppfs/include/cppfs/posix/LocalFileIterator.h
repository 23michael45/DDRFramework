
#pragma once


#include <memory>
#include <dirent.h>
#include <sys/stat.h>

#include <cppfs/AbstractFileIteratorBackend.h>


namespace cppfs
{


class LocalFileSystem;


/**
*  @brief
*    File iterator for the local file system
*/
class CPPFS_API LocalFileIterator : public AbstractFileIteratorBackend
{
public:
    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] fs
    *    File system that created this iterator
    *  @param[in] path
    *    Path to file or directory
    */
    LocalFileIterator(std::shared_ptr<LocalFileSystem> fs, const std::string & path);

    /**
    *  @brief
    *    Constructor
    *
    *  @param[in] fs
    *    File system that created this iterator
    *  @param[in] path
    *    Path to file or directory
    */
    LocalFileIterator(std::shared_ptr<LocalFileSystem> fs, std::string && path);

    /**
    *  @brief
    *    Destructor
    */
    virtual ~LocalFileIterator();

    // Virtual AbstractFileIteratorBackend functions
    virtual std::unique_ptr<AbstractFileIteratorBackend> clone() const override;
    virtual AbstractFileSystem * fs() const override;
    virtual bool valid() const override;
    virtual std::string path() const override;
    virtual int index() const override;
    virtual std::string name() const override;
    virtual void next() override;


protected:
    void readNextEntry();


protected:
    std::shared_ptr<LocalFileSystem>   m_fs;    ///< File system that created this iterator
    std::string                        m_path;  ///< Path to file or directory
    DIR                              * m_dir;   ///< Directory handle
    struct dirent                    * m_entry; ///< Current directory entry
    int                                m_index; ///< Index of the current entry
};


} // namespace cppfs
