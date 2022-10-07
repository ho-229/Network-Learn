/**
 * @author Ho 229
 * @date 2022/10/7
 */

#include "epoll.h"

#include <iostream>

#if defined (OS_WINDOWS)

Epoll::Epoll()
{
//    if (m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
//        m_iocp == nullptr)
//        throw std::runtime_error("CreateIoCompletionPort failed.");
    m_wepoll = epoll_create1(1);
}

Epoll::~Epoll()
{

}

void Epoll::insert(AbstractSocket* const socket, bool exclusive)
{
    epoll_event event{EPOLLIN | EPOLLRDHUP, socket};
    std::cerr << epoll_ctl(m_wepoll, EPOLL_CTL_ADD, socket->descriptor(), &event) << "\n";
    ++m_count;
//    static_cast<void>(exclusive);

//    Socket baseSocket = INVALID_SOCKET;
//    DWORD ret;

//    if(m_info.find(socket) != m_info.end() &&   // Socket already exist
//        WSAIoctl(socket->descriptor(),          // Get base socket
//                 SIO_BASE_HANDLE,
//                 nullptr, 0,
//                 &baseSocket, sizeof(Socket),
//                 &ret, nullptr, nullptr) == SOCKET_ERROR)
//        return;
}

void Epoll::erase(AbstractSocket* const socket)
{
    epoll_ctl(m_wepoll, EPOLL_CTL_DEL, socket->descriptor(), nullptr);
    --m_count;
}

void Epoll::epoll(std::vector<AbstractSocket*>& events,
    std::vector<AbstractSocket*>& errorEvents)
{
    int ret = -1;
    if((ret = epoll_wait(m_wepoll, m_eventBuf, EPOLL_MAX_EVENTS, EPOLL_WAIT_TIMEOUT)) <= 0)
        return;

    epoll_event *item = nullptr;
    for(int i = 0; i < ret; ++i)
    {
        item = m_eventBuf + i;

        if(item->events & EPOLLERR || item->events & EPOLLHUP || item->events & EPOLLRDHUP)
            errorEvents.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
        else if(item->events & EPOLLIN)
            events.emplace_back(reinterpret_cast<AbstractSocket *>(item->data.ptr));
    }
}

size_t Epoll::count() const
{
    return m_count;
}

#endif
