/**
 * @author Ho 229
 * @date 2021/11/28
 */

#include "sharedfilepool.h"

#ifdef _WIN32
# include <Windows.h>
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
        File file = {};
        fs::path filePath(m_root + fileName);

#ifdef _WIN32
        if((file = CreateFile(reinterpret_cast<LPCSTR>(filePath.c_str()),
                               GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               nullptr)) == INVALID_HANDLE_VALUE)
            return {};
#else
        if((file = open(filePath.c_str(), O_RDONLY)) < 0)
            return {};
#endif

        WriteLock lock(m_mutex);
        const FileInfo ret{file, static_cast<size_t>(fs::file_size(filePath))};
        return {m_pool.emplace(fileName, ret).first->second};
    }

    return {it->second};
}

SharedFilePool::~SharedFilePool()
{
    for(const auto &[key, value] : m_pool)
#ifdef _WIN32
        CloseHandle(value.file);
#else   // Unix
        ::close(value.file);
#endif
}
