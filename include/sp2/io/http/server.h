#ifndef SP2_IO_HTTP_SERVER_H
#define SP2_IO_HTTP_SERVER_H

#include <sp2/string.h>
#include <sp2/updatable.h>
#include <sp2/io/network/tcpListener.h>
#include <sp2/io/network/tcpSocket.h>
#include <list>
#include <thread>
#include <mutex>
#include <unordered_map>


namespace sp {
namespace io {
namespace http {

class Request
{
public:
    string method;
    string path;
    string post_data;
    std::unordered_map<string, string> headers;
};

/**
    Basic HTTP webserver.
    Runs a single threaded webserver which can handle:
        * File hosting
        * APIs
        * Websockets
    Protocol and file handling is done on a seperate thread, while URL handlers and Websocket handlers are processed on the main thread.
    
    Note that the websockets implementation is limited to small text messages. Without fragmentation/continuation frames.
 */
class Server : public Updatable
{
public:
    Server(int port_nr=80);
    
    void setStaticFilePath(const string& static_file_path);
    void addURLHandler(const string& url, std::function<string(const Request&)> func);
    void addWebsocketHandler(const string& url, std::function<void(const string& data)> func);
    void broadcastToWebsockets(const string& url, const string& data);
private:
    string static_file_path;

    void handlerThread();
    virtual void onUpdate(float delta) override;

    std::thread handler_thread;
    std::recursive_mutex mutex;
    std::map<string, std::function<string(const Request&)>> http_handlers;
    std::map<string, std::function<void(const string& data)>> websocket_handlers;

    sp::io::network::TcpListener listen_socket;
    
    class Connection : sp::NonCopyable
    {
    public:
        Connection(Server& server);

        sp::io::network::TcpSocket socket;
        string buffer;
        Server& server;
        
        Request request;
        bool request_pending = false;
        string websocket_received_fragment;
        std::vector<string> websocket_received_pending;

        bool update();
        void handleRequest(const Request& request);
        void startHttpReply(int reply_code);
        void httpChunk(const string& data);
        void sendWebsocketTextPacket(const string& data);
        
        enum class State
        {
            HTTPRequest,
            Websocket
        } state;
    };
    
    std::list<Connection> connections;
};

};//namespace http
};//namespace io
};//namespace sp

#endif//SP2_IO_HTTP_SERVER_H
