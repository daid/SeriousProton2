#include <sp2/io/network/socketBase.h>

#ifdef __WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
static constexpr int flags = 0;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
static constexpr int flags = MSG_NOSIGNAL;
#endif


namespace sp {
namespace io {
namespace network {


void SocketBase::setBlocking(bool blocking)
{
    this->blocking = blocking;
    if (handle == -1)
        return;

#ifdef __WIN32
   unsigned long mode = blocking ? 0 : 1;
   ::ioctlsocket(handle, FIONBIO, &mode);
#else
    int flags = ::fcntl(handle, F_GETFL, 0);
    if (blocking)
        flags &=~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    ::fcntl(handle, F_SETFL, flags);
#endif
}

bool SocketBase::isLastErrorNonBlocking()
{
#ifdef __WIN32
    int error = ::WSAGetLastError();
    if (error == WSAEWOULDBLOCK || error == WSAEALREADY)
        return true;
#else
    if (errno == EAGAIN || errno == EINPROGRESS || errno == EWOULDBLOCK)
        return true;
#endif
    return false;
}

};//namespace network
};//namespace io
};//namespace sp

