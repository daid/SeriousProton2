#ifndef SP2_IO_HTTP_WEBSOCKET_H
#define SP2_IO_HTTP_WEBSOCKET_H

#include <sp2/updatable.h>
#include <sp2/io/network/tcpSocket.h>

namespace sp {
namespace io {
namespace http {

class Websocket : public sp::Updatable
{
public:
    Websocket();
    ~Websocket();

    void setMessageCallback(std::function<void(const string&)> callback);
    
    ///Connect to a http server with the websocket protocol. The URL should be composed like:
    /// ws://server.com/path
    ///     Returns true when the initial connection is done. But actual protocol negotiation is still happening.
    ///     isConnected will become true when negotiation is done.
    bool connect(const string& url);
    void close();

    bool isConnected();
    bool isConnecting();

    void send(const string& message);
    
    virtual void onUpdate(float delta) override;
private:
    enum class State
    {
        Disconnected,
        Connecting,
        Operational,
    } state;
    
    string websock_key;
    sp::io::network::TcpSocket socket;
    string buffer;
    string received_fragment;
    std::function<void(const string&)> callback;
};

};//namespace http
};//namespace io
};//namespace sp

#endif//SP2_IO_HTTP_WEBSOCKET_H
