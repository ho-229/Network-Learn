/**
 * @author Ho 229
 * @date 2021/11/28
 */

#include "sharedfilepool.h"

#ifdef __linux__    // Only for Linux

#include <fcntl.h>

SharedFilePool::SharedFilePool(const std::string &root)
{
    m_root = root;
}

std::optional<FileInfo> SharedFilePool::get(const std::string &fileName)
{
    decltype (m_pool)::const_iterator it;
    {
        ReadLock lock(m_mutex);
        it = m_pool.find(fileName);
    }

    if(it == m_pool.end())  // Open file
    {
        int fd = -1;
        fs::path newPath(m_root + fileName);

        if((fd = open(newPath.c_str(), O_RDONLY)) < 0)
            return {};

        WriteLock lock(m_mutex);
        const FileInfo ret{fd, fs::file_size(newPath)};
        return {m_pool.emplace(fileName, ret).first->second};
    }

    return {it->second};
}

#endif
