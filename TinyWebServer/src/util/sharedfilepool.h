/**
 * @author Ho 229
 * @date 2021/11/28
 */

#ifndef SHAREDFILEPOOL_H
#define SHAREDFILEPOOL_H

#include <string>
#include <optional>
#include <filesystem>
#include <shared_mutex>
#include <unordered_map>

namespace fs = std::filesystem;

struct FileInfo
{
    int fd;
    size_t fileSize;
};

class SharedFilePool
{
public:
    explicit SharedFilePool(const std::string &root);
    ~SharedFilePool();

    std::string root() const { return m_root; }

    std::optional<FileInfo> get(const std::string &fileName);

private:
    std::unordered_map<std::string, FileInfo> m_pool;

    std::string m_root;

    std::shared_mutex m_mutex;

    typedef std::shared_lock<std::shared_mutex> ReadLock;
    typedef std::lock_guard<std::shared_mutex> WriteLock;
};

#endif // SHAREDFILEPOOL_H
