#include <sp2/io/network/tcpSocket.h>

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
static constexpr int flags = MSG_NOSIGNAL;
#endif


namespace sp {
namespace io {
namespace network {


TcpSocket::TcpSocket()
{
}

TcpSocket::TcpSocket(TcpSocket&& socket)
{
    handle = socket.handle;
    send_queue = std::move(socket.send_queue);
    blocking = socket.blocking;

    socket.handle = -1;
    socket.send_queue.clear();
}

TcpSocket::~TcpSocket()
{
    close();
}

bool TcpSocket::connect(const Address& host, int port)
{
    if (isConnected())
        close();
    
    for(const auto& addr_info : host.addr_info)
    {
        handle = ::socket(addr_info.family, SOCK_STREAM, 0);
        if (handle < 0)
            return false;
        if (::connect(handle, (const sockaddr*)addr_info.addr.data(), addr_info.addr.length()) == 0)
        {
            setBlocking(blocking);
            return true;
        }
        close();
    }
    return false;
}

void TcpSocket::close()
{
    if (isConnected())
    {
#ifdef __WIN32
        closesocket(handle);
#else
        ::close(handle);
#endif
        handle = -1;
        send_queue.clear();
    }
}

bool TcpSocket::isConnected()
{
    return handle != -1;
}

void TcpSocket::send(const void* data, size_t size)
{
    if (!isConnected())
        return;
    if (sendSendQueue())
    {
        send_queue += std::string((const char*)data, size);
        return;
    }

    for(size_t done = 0; done < size; )
    {
        int result = ::send(handle, ((const char*)data) + done, size - done, flags);
        if (result < 0)
        {
            if (!isLastErrorNonBlocking())
                close();
            else
                send_queue += std::string(((const char*)data) + done, size - done);
            return;
        }
        done += result;
    }
}

size_t TcpSocket::receive(void* data, size_t size)
{
    sendSendQueue();
    
    if (!isConnected())
        return 0;
    
    int result = ::recv(handle, (char*)data, size, flags);
    if (result < 0)
    {
        result = 0;
        if (!isLastErrorNonBlocking())
            close();
    }
    return result;
}

//void send(const DataBuffer& buffer);
//bool receive(DataBuffer& buffer);

void TcpSocket::setBlocking(bool blocking)
{
    this->blocking = blocking;
    if (!isConnected())
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

bool TcpSocket::isLastErrorNonBlocking()
{
#ifdef __WIN32
    int error = WSAGetLastError();
    if (error == WSAEWOULDBLOCK || error == WSAEALREADY)
        return true;
#else
    if (errno == EAGAIN || errno == EINPROGRESS || errno == EWOULDBLOCK)
        return true;
#endif
    return false;
}

bool TcpSocket::sendSendQueue()
{
    if (send_queue.size() < 1)
        return false;
    
    int result;
    do
    {
        result = ::send(handle, (const char*)send_queue.data(), send_queue.size(), flags);
        if (result < 0)
        {
            result = 0;
            if (!isLastErrorNonBlocking())
                close();
        }
        if (result > 0)
            send_queue = send_queue.substr(result);
    } while(result > 0 && send_queue.size() > 0);
    
    return send_queue.size() > 0;
}

};//namespace network
};//namespace io
};//namespace sp
