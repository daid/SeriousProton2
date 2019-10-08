#ifndef SP2_IO_NETWORK_UDP_SOCKET_H
#define SP2_IO_NETWORK_UDP_SOCKET_H

#include <sp2/nonCopyable.h>
#include <sp2/string.h>
#include <sp2/io/network/address.h>
#include <sp2/io/network/socketBase.h>
#include <sp2/io/dataBuffer.h>


namespace sp {
namespace io {
namespace network {


class UdpSocket : public SocketBase
{
public:
    UdpSocket();
    UdpSocket(UdpSocket&& socket);
    ~UdpSocket();

    bool bind(int port);
    bool joinMulticast(int group_nr);
    void close();

    bool send(const void* data, size_t size, const Address& address, int port);
    size_t receive(void* data, size_t size, Address& address, int& port);

    bool send(const DataBuffer& buffer, const Address& address, int port);
    bool receive(DataBuffer& buffer, Address& address, int& port);

    bool sendMulticast(const void* data, size_t size, int group_nr, int port);
    bool sendMulticast(const DataBuffer& buffer, int group_nr, int port);
private:
    bool createSocket();

    bool socket_is_ipv6;
};

}//namespace network
}//namespace io
}//namespace sp

#endif//SP2_IO_NETWORK_TCP_SOCKET_H
