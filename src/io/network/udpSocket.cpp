#include <sp2/io/network/udpSocket.h>

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
#include <netdb.h>
static constexpr int flags = MSG_NOSIGNAL;
#endif


namespace sp {
namespace io {
namespace network {


UdpSocket::UdpSocket()
{
}

UdpSocket::UdpSocket(UdpSocket&& socket)
{
    handle = socket.handle;
    blocking = socket.blocking;

    socket.handle = -1;
}

UdpSocket::~UdpSocket()
{
    close();
}

bool UdpSocket::bind(int port)
{
    handle = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (handle != -1)
    {
        socket_is_ipv6 = true;
        int optval = 0;
        ::setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&optval, sizeof(int));

        struct sockaddr_in6 server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_addr = in6addr_any;
        server_addr.sin6_port = htons(port);

        if (::bind(handle, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            close();
            return false;
        }
    }
    else
    {
        handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle == -1)
            return false;
        socket_is_ipv6 = false;

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(port);

        if (::bind(handle, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
            close();
            return false;
        }
    }
    return true;
}

void UdpSocket::close()
{
    if (handle != -1)
    {
#ifdef __WIN32
        closesocket(handle);
#else
        ::close(handle);
#endif
        handle = -1;
    }
}

bool UdpSocket::send(const void* data, size_t size, const Address& address, int port)
{
    if (handle == -1)
    {
        handle = ::socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        socket_is_ipv6 = true;
        if (handle == -1)
        {
            handle = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            socket_is_ipv6 = false;
            if (handle == -1)
                return false;
        }
    }
    
    if (socket_is_ipv6)
    {
        struct sockaddr_in6 server_addr;
        bool is_set = false;
        memset(&server_addr, 0, sizeof(server_addr));
        for(auto& addr_info : address.addr_info)
        {
            if (addr_info.addr.length() == sizeof(server_addr))
            {
                memcpy(&server_addr.sin6_addr, addr_info.addr.data(), addr_info.addr.length());
                is_set = true;
                break;
            }
        }
        
        if (is_set)
        {
            server_addr.sin6_family = AF_INET6;
            server_addr.sin6_port = htons(port);

            int result = ::sendto(handle, (const char*)data, size, flags, (const sockaddr*)&server_addr, sizeof(server_addr));
            return result == int(size);
        }
    }

    struct sockaddr_in server_addr;
    bool is_set = false;
    memset(&server_addr, 0, sizeof(server_addr));
    for(auto& addr_info : address.addr_info)
    {
        if (addr_info.addr.length() == sizeof(server_addr))
        {
            memcpy(&server_addr.sin_addr, addr_info.addr.data(), addr_info.addr.length());
            is_set = true;
            break;
        }
    }

    if (is_set)
    {
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        int result = ::sendto(handle, (const char*)data, size, flags, (const sockaddr*)&server_addr, sizeof(server_addr));
        return result == int(size);
    }
    return false;
}

size_t UdpSocket::receive(void* data, size_t size, Address& address, int& port)
{
    if (handle == -1)
        return 0;
    address.addr_info.clear();
    if (socket_is_ipv6)
    {
        struct sockaddr_in6 from_addr;
        memset(&from_addr, 0, sizeof(from_addr));
        socklen_t from_addr_len = sizeof(from_addr);
        int result = ::recvfrom(handle, (char*)data, size, flags, (sockaddr*)&from_addr, &from_addr_len);
        if (result >= 0)
        {
            if (from_addr_len == sizeof(from_addr))
            {
                char buffer[128];
                ::getnameinfo((const sockaddr*)&from_addr, from_addr_len, buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                address.addr_info.emplace_back(AF_INET6, buffer, &from_addr, from_addr_len);
                port = ntohs(from_addr.sin6_port);
            }
            else if (from_addr_len == sizeof(struct sockaddr_in))
            {
                char buffer[128];
                ::getnameinfo((const sockaddr*)&from_addr, from_addr_len, buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                address.addr_info.emplace_back(AF_INET, buffer, &from_addr, from_addr_len);
                port = ntohs(((struct sockaddr_in*)&from_addr)->sin_port);
            }
            return result;
        }
    }
    else
    {
        struct sockaddr_in from_addr;
        memset(&from_addr, 0, sizeof(from_addr));
        socklen_t from_addr_len = sizeof(from_addr);
        int result = ::recvfrom(handle, (char*)data, size, flags, (sockaddr*)&from_addr, &from_addr_len);
        if (result >= 0)
        {
            if (from_addr_len == sizeof(from_addr))
            {
                char buffer[128];
                ::getnameinfo((const sockaddr*)&from_addr, from_addr_len, buffer, sizeof(buffer), nullptr, 0, NI_NUMERICHOST);
                address.addr_info.emplace_back(AF_INET, buffer, &from_addr, from_addr_len);
                port = ntohs(from_addr.sin_port);
            }
            return result;
        }
    }
    return 0;
}

//void UdpSocket::send(const DataBuffer& buffer, const Address& address, int port);
//bool UdpSocket::receive(DataBuffer& buffer, Address& address, int& port);

};//namespace network
};//namespace io
};//namespace sp
