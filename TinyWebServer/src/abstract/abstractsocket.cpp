/**
 * @author Ho 229
 * @date 2021/11/21
 */

#include "abstractsocket.h"

thread_local std::shared_ptr<char[]> AbstractSocket::buffer(
    new char[SOCKET_BUF_SIZE]());

bool AbstractSocket::sendStream(std::istream * const stream, size_t count)
{
    if(!stream)
        return false;

    static thread_local std::shared_ptr<char[]> filebuf(new char[SOCKET_BUF_SIZE]());

    if(count)
    {
        size_t leftSize = count;
        int writtenSize = 0;

        while(leftSize > 0 && !stream->eof())
        {
            stream->read(filebuf.get(),
                         leftSize > SOCKET_BUF_SIZE ? SOCKET_BUF_SIZE : leftSize);
            if((writtenSize = this->write(filebuf.get(), int(stream->gcount()))) <= 0)
                return false;

            leftSize -= size_t(writtenSize);
        }
    }
    else
    {
        while(!stream->eof())
        {
            stream->read(filebuf.get(), SOCKET_BUF_SIZE);
            this->write(filebuf.get(), int(stream->gcount()));
        }
    }

    return true;
}
