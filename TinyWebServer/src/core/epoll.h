/**
 * @author Ho 229
 * @date 2021/8/20
 */

#ifndef EPOLL_H
#define EPOLL_H

#include "../define.h"
#include "../abstract/abstractsocket.h"

#include <vector>

#define EPOLL_THREAD_SAFE 0

# include <atomic>

#if defined (OS_WINDOWS)
# include <unordered_map>
# include <mutex>
#elif defined (OS_LINUX)
# include <sys/epoll.h>
#else
# include <sys/event.h>
# include <sys/types.h>
# include <sys/time.h>
# include <unistd.h>
#endif

class Epoll
{
public:
    explicit Epoll();
    ~Epoll();

    void insert(AbstractSocket *const socket, bool exclusive = false);
    void erase(AbstractSocket *const socket);

    void epoll(std::vector<AbstractSocket *> &events,
               std::vector<AbstractSocket *> &errorEvents);

    size_t count() const { return m_count; }

private:
    std::atomic_uint m_count = 0;
#if defined (OS_WINDOWS)    // Windows IOCP
    struct AfdPollInfo
    {
        LARGE_INTEGER timeout;
        ULONG handleNumber;
        ULONG exclusive;
        struct AfdPollHandleInfo
        {
            HANDLE handle;
            ULONG event;
            NTSTATUS status;
        } handles[1];
    };

    enum class EpollStatus
    {
        IDLE,
        PENDING,
        CANCELLED
    };

    struct EpollInfo
    {
        OVERLAPPED overlapped;
        AfdPollInfo pollInfo;
        EpollStatus pollStatus;
        UINT32 pendingEvents;
        bool pendingDelete;
        Socket peerSocket;
    };

    HANDLE m_iocp = INVALID_HANDLE_VALUE;
    std::unordered_map<AbstractSocket *, EpollInfo> m_info;
#elif defined (OS_LINUX)    // Linux epoll
    int m_epoll = 0;
    epoll_event m_eventBuf[EPOLL_MAX_EVENTS];
#elif define (OS_UNIX)      // Unix kqueue
    int m_kqueue = 0;
    struct kevent m_eventBuf[EPOLL_MAX_EVENTS];
#endif
};

#endif // EPOLL_H
