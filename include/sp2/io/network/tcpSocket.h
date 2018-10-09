#ifndef SP2_IO_NETWORK_TCP_SOCKET_H
#define SP2_IO_NETWORK_TCP_SOCKET_H

#include <sp2/io/network/address.h>
#include <sp2/io/network/socketBase.h>
#include <sp2/io/dataBuffer.h>


namespace sp {
namespace io {
namespace network {


class TcpSocket : public SocketBase
{
public:
    TcpSocket();
    TcpSocket(TcpSocket&& socket);
    ~TcpSocket();

    bool connect(const Address& host, int port);
    void close();

    bool isConnected();

    void send(const void* data, size_t size);
    size_t receive(void* data, size_t size);

    void send(const io::DataBuffer& buffer);
    bool receive(io::DataBuffer& buffer);

private:
    bool sendSendQueue();

    std::string send_queue;
    
    friend class TcpListener;
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_SOCKET_H
