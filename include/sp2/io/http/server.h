#ifndef SP2_IO_HTTP_SERVER_H
#define SP2_IO_HTTP_SERVER_H

#include <sp2/string.h>
#include <sp2/updatable.h>
#include <SFML/Network.hpp>
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
    
    Note that the websockets implementation is limited to small text messages, less then 4k in size. Without fragmentation/continuation frames.
 */
class Server : public Updatable
{
public:
    Server(int port_nr=80);
    
    void setStaticFilePath(string static_file_path);
    void addHandler(string url, std::function<string(const Request&)> func);
    void addWebsocketHandler(string url, std::function<void(const string& data)> func);
    void sendToWebsockets(string url, string data);
private:
    string static_file_path;

    void handlerThread();
    virtual void onUpdate(float delta) override;

    std::thread handler_thread;
    std::mutex mutex;
    std::map<string, std::function<string(const Request&)>> http_handlers;
    std::map<string, std::function<void(const string& data)>> websocket_handlers;

    sf::TcpListener listen_socket;
    
    class Connection : sp::NonCopyable
    {
    public:
        Connection(Server& server);

        sf::TcpSocket socket;
        string buffer;
        Server& server;
        
        Request request;
        bool request_pending = false;
        std::vector<string> websock_pending;

        bool update();
        void handleRequest(const Request& request);
        void startHttpReply(int reply_code);
        void httpChunk(const string data);
        void sendWebsocketTextPacket(string data);
        
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
