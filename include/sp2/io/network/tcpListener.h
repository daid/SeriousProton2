#ifndef SP2_IO_NETWORK_TCP_LISTENER_H
#define SP2_IO_NETWORK_TCP_LISTENER_H

#include <sp2/io/network/address.h>
#include <sp2/io/network/socketBase.h>


namespace sp {
namespace io {
namespace network {


class TcpSocket;
class TcpListener : public SocketBase
{
public:
    TcpListener();
    ~TcpListener();

    bool listen(int port);
    void close();
    
    bool isListening();

    bool accept(TcpSocket& socket);
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_LISTENER_H
