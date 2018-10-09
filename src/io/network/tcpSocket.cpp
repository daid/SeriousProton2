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
#include <string.h>
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
        if (addr_info.family == AF_INET && sizeof(struct sockaddr_in) == addr_info.addr.length())
        {
            struct sockaddr_in server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            memcpy(&server_addr, addr_info.addr.data(), addr_info.addr.length());
            server_addr.sin_port = ::htons(port);
            if (::connect(handle, (const sockaddr*)&server_addr, sizeof(server_addr)) == 0)
            {
                setBlocking(blocking);
                return true;
            }
        }
        if (addr_info.family == AF_INET6 && sizeof(struct sockaddr_in6) == addr_info.addr.length())
        {
            struct sockaddr_in6 server_addr;
            memset(&server_addr, 0, sizeof(server_addr));
            memcpy(&server_addr, addr_info.addr.data(), addr_info.addr.length());
            server_addr.sin6_port = ::htons(port);
            if (::connect(handle, (const sockaddr*)&server_addr, sizeof(server_addr)) == 0)
            {
                setBlocking(blocking);
                return true;
            }
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

void TcpSocket::send(const io::DataBuffer& buffer)
{
    
}

bool TcpSocket::receive(io::DataBuffer& buffer)
{
    
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
