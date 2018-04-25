#ifndef SP2_IO_NETWORK_TCP_SOCKET_H
#define SP2_IO_NETWORK_TCP_SOCKET_H

#include <sp2/nonCopyable.h>
#include <sp2/io/network/address.h>


namespace sp {
namespace io {
namespace network {


class TcpSocket : NonCopyable
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

    //void send(const DataBuffer& buffer);
    //bool receive(DataBuffer& buffer);

    void setBlocking(bool blocking);

private:
    bool isLastErrorNonBlocking();
    bool sendSendQueue();

    std::string send_queue;
    bool blocking = true;
    int handle = -1;
    
    friend class TcpListener;
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_SOCKET_H
