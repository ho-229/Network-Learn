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
    : m_root(fs::path(root).lexically_normal().string())
{

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

        {
            ReadLock lock(m_mutex);
            if(m_invalidPaths.find(filePath.string()) != m_invalidPaths.end())
                return {};
        }

#ifdef _WIN32
        if((file = CreateFile(reinterpret_cast<LPCSTR>(filePath.string().c_str()),
                               GENERIC_READ, FILE_SHARE_READ, nullptr,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                               nullptr)) == INVALID_HANDLE_VALUE)
#else
        if(!fs::is_regular_file(filePath) || (file = open(filePath.c_str(), O_RDONLY)) < 0)
#endif
        {
            WriteLock lock(m_mutex);

            if(m_invalidPaths.size() >= MAX_SHARED_FILE)
                m_invalidPaths.erase(m_invalidPaths.begin());

            m_invalidPaths.insert(filePath.string());
            return {};
        }

        WriteLock lock(m_mutex);
        const FileInfo ret{file, static_cast<size_t>(fs::file_size(filePath)),
                           filePath.extension().string()};

        if(m_pool.size() >= MAX_SHARED_FILE)
            m_pool.erase(m_pool.begin());

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
