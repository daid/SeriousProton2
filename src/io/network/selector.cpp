#include <sp2/io/network/selector.h>
#include <algorithm>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <errno.h>
#endif


namespace sp {
namespace io {
namespace network {


class Selector::SelectorData
{
public:
    fd_set sockets;
    fd_set ready;
    int max_handle;
};

Selector::Selector()
: data(new SelectorData())
{
    FD_ZERO(&data->sockets);
    FD_ZERO(&data->ready);
    data->max_handle = 0;
}

Selector::Selector(const Selector& other)
: data(new SelectorData(*other.data))
{
}

Selector::~Selector()
{
    delete data;
}

Selector& Selector::operator =(const Selector& other)
{
    *data = *other.data;
    return *this;
}

void Selector::add(SocketBase& socket)
{
    if (socket.handle != -1)
    {
        FD_SET(socket.handle, &data->sockets);
        data->max_handle = std::max(socket.handle, data->max_handle);
    }
}

void Selector::remove(SocketBase& socket)
{
    if (socket.handle != -1)
    {
        FD_CLR(socket.handle, &data->sockets);
        FD_CLR(socket.handle, &data->ready);
    }
}

void Selector::wait(int timeout_ms)
{
    data->ready = data->sockets;
    
    if (timeout_ms >= 0)
    {
        timeval timeout;
        timeout.tv_sec  = timeout_ms / 1000;
        timeout.tv_usec = (timeout_ms % 1000) * 1000;
        ::select(data->max_handle + 1, &data->ready, nullptr, nullptr, &timeout);
    }
    else
    {
        ::select(data->max_handle + 1, &data->ready, nullptr, nullptr, nullptr);
    }
}

bool Selector::isReady(SocketBase& socket)
{
    if (socket.handle != -1)
        return FD_ISSET(socket.handle, &data->ready);
    return false;
}

};//namespace network
};//namespace io
};//namespace sp
