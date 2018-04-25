#ifndef SP2_IO_NETWORK_UDP_SOCKET_H
#define SP2_IO_NETWORK_UDP_SOCKET_H

#include <sp2/nonCopyable.h>
#include <sp2/string.h>
#include <sp2/io/network/address.h>

namespace sp {
namespace io {
namespace network {


class UdpSocket : NonCopyable
{
public:
    UdpSocket();
    UdpSocket(UdpSocket&& socket);
    ~UdpSocket();
    
    void send(const DataBuffer& buffer, const Address& address, int port);
    bool receive(DataBuffer& buffer, Address& address, int& port);

    void setBlocking(bool blocking);
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_SOCKET_H
