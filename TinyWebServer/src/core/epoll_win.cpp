/**
 * @author Ho 229
 * @date 2022/10/7
 */

#include "epoll.h"

#if defined (OS_WINDOWS)

Epoll::Epoll()
{
    if (m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
        m_iocp == 0)
        throw std::runtime_error("CreateIoCompletionPort failed.");
}

Epoll::~Epoll()
{

}

void Epoll::insert(AbstractSocket* const socket, bool exclusive)
{

}

void Epoll::erase(AbstractSocket* const socket)
{

}

void Epoll::epoll(std::vector<AbstractSocket*>& events,
    std::vector<AbstractSocket*>& errorEvents)
{
    
}

#endif
