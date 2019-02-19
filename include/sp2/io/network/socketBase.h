#ifndef SP2_IO_NETWORK_SOCKET_BASE_H
#define SP2_IO_NETWORK_SOCKET_BASE_H

#include <sp2/io/network/socketBase.h>
#include <sp2/nonCopyable.h>


namespace sp {
namespace io {
namespace network {


class SocketBase : NonCopyable
{
public:
    void setBlocking(bool blocking);

protected:
    bool isLastErrorNonBlocking();

    int handle = -1;
    bool blocking = true;
    
    friend class Selector;
};

};//namespace network
};//namespace io
};//namespace sp

#endif//SP2_IO_NETWORK_SOCKET_BASE_H
