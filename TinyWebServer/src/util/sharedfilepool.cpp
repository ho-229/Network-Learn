/**
 * @author Ho 229
 * @date 2021/11/28
 */

#include "sharedfilepool.h"

// Only for Linux

#ifdef _WIN32
#else
# include <fcntl.h>
# include <unistd.h>
#endif

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

#ifdef _WIN32
        if(!(fd = fs::is_regular_file(newPath)))     // TODO
            return {};
#else
        if((fd = open(newPath.c_str(), O_RDONLY)) < 0)
            return {};
#endif

        WriteLock lock(m_mutex);
        const FileInfo ret{fd, static_cast<size_t>(fs::file_size(newPath))};
        return {m_pool.emplace(fileName, ret).first->second};
    }

    return {it->second};
}

SharedFilePool::~SharedFilePool()
{
#ifdef _WIN32
#else
    for(const auto&[key, value] : m_pool)
        close(value.fd);
#endif
}
