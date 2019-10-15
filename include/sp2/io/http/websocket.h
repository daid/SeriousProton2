#ifndef SP2_IO_HTTP_WEBSOCKET_H
#define SP2_IO_HTTP_WEBSOCKET_H

#include <sp2/nonCopyable.h>
#include <sp2/io/network/tcpSocket.h>
#include <unordered_map>


namespace sp {
namespace io {
namespace http {

class Websocket : public sp::NonCopyable
{
public:
    Websocket();
    Websocket(Websocket&& other);
    ~Websocket();

    Websocket& operator=(Websocket&& other);

    ///Connect to a http server with the websocket protocol. The URL should be composed like:
    /// ws://server.com/path
    ///     Returns true when the initial connection is done. But actual protocol negotiation is still happening.
    ///     isConnected will become true when negotiation is done.
    bool connect(const string& url);
    void close();

    void setHeader(const string& key, const string& value);

    bool isConnected();
    bool isConnecting();

    void send(const string& message);
    bool receive(string& message);

    void send(const io::DataBuffer& data_buffer);
    bool receive(io::DataBuffer& data_buffer);
private:
    enum class State
    {
        Disconnected,
        Connecting,
        Operational,
    } state = State::Disconnected;

    string websock_key;
    sp::io::network::TcpSocket socket;
    std::vector<uint8_t> buffer;
    std::vector<uint8_t> received_fragment;
    std::unordered_map<string, string> headers;
};

}//namespace http
}//namespace io
}//namespace sp

#endif//SP2_IO_HTTP_WEBSOCKET_H
