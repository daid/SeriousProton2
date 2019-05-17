#ifndef SP2_IO_HTTP_SERVER_H
#define SP2_IO_HTTP_SERVER_H

#include <sp2/string.h>
#include <sp2/updatable.h>
#include <sp2/io/network/tcpListener.h>
#include <sp2/io/network/tcpSocket.h>
#include <list>
#include <thread>
#include <mutex>
#include <chrono>
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
    
    //Set the path on the filesystem where statics files are read from.
    //  Note: This does not use the ResourceProvider system.
    void setStaticFilePath(const string& static_file_path);
    //Add a callback function to handle a specific URL request.
    // The URL should be prefixed with a "/", the return value of the callback is send back as data to the browser.
    void addURLHandler(const string& url, std::function<string(const Request&)> func);
    //Add a simple websocket handler, this handler will process any websocket message from any websocket connected to a specific URL.
    //  No distinction or state between connections is made.
    //  URLs should start with a "/"
    void addWebsocketHandler(const string& url, std::function<void(const string& data)> func);
    //Send a message to all connected websockets to a specific URL endpoint.
    //  No distinction is made between websockets, all are equal. Useful to distribute game state to all websockets.
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
        std::chrono::steady_clock::time_point last_received_data_time;
        string buffer;
        Server& server;
        
        Request request;
        bool request_pending = false;
        string websocket_received_fragment;
        std::vector<string> websocket_received_pending;

        bool processIncommingData();
        bool handleTimeout();
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
