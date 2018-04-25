#ifndef SP2_IO_NETWORK_TCP_LISTENER_H
#define SP2_IO_NETWORK_TCP_LISTENER_H

#include <sp2/nonCopyable.h>
#include <sp2/string.h>
#include <sp2/io/network/address.h>


namespace sp {
namespace io {
namespace network {


class TcpSocket;
class TcpListener : NonCopyable
{
public:
    TcpListener();
    ~TcpListener();

    bool listen(int port);
    void close();
    
    bool isListening();

    bool accept(TcpSocket& socket);

    void setBlocking(bool blocking);

private:
    bool isLastErrorNonBlocking();

    int handle = -1;
    bool blocking = true;
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_TCP_LISTENER_H
