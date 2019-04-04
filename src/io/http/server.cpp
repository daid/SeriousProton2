#include <sp2/io/http/server.h>
#include <sp2/io/network/selector.h>
#include <sp2/stringutil/sha1.h>
#include <sp2/stringutil/convert.h>
#include <sp2/logging.h>

#include <string.h>

namespace sp {
namespace io {
namespace http {

Server::Server(int port_nr)
{
    if (!listen_socket.listen(port_nr))
    {
        LOG(Error, "Failed to listen on port:", port_nr, "for http server");
        return;
    }

    std::thread thread([this]() { handlerThread(); });
    std::swap(handler_thread, thread);
}

void Server::setStaticFilePath(string static_file_path)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    if (!static_file_path.endswith("/"))
        static_file_path += "/";
    this->static_file_path = static_file_path;
}

void Server::addHandler(string url, std::function<string(const Request&)> func)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    http_handlers[url] = func;
}

void Server::addWebsocketHandler(string url, std::function<void(const string& data)> func)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    websocket_handlers[url] = func;
}

void Server::sendToWebsockets(string url, string data)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    
    for(Connection& connection : connections)
    {
        if (connection.state == Connection::State::Websocket && connection.request.path == url)
            connection.sendWebsocketTextPacket(data);
    }
}

void Server::handlerThread()
{
    sp::io::network::Selector selector;
    selector.add(listen_socket);
    while(true)
    {
        selector.wait(1000);

        std::lock_guard<std::recursive_mutex> lock(mutex);
        if (selector.isReady(listen_socket))
        {
            connections.emplace_back(*this);
            Connection& connection = connections.back();
            listen_socket.accept(connection.socket);
            selector.add(connection.socket);
        }
        for(auto it = connections.begin(); it != connections.end();)
        {
            Connection& connection = *it;
            if (selector.isReady(connection.socket))
            {
                if (connection.update())
                {
                    it++;
                }
                else
                {
                    selector.remove(connection.socket);
                    it = connections.erase(it);
                }
            }
            else
            {
                it++;
            }
        }
    }
}

void Server::onUpdate(float delta)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);

    for(Connection& connection : connections)
    {
        if (connection.request_pending)
        {
            string reply = http_handlers[connection.request.path](connection.request);
            connection.startHttpReply(200);
            if (reply.size() > 0)
                connection.httpChunk(reply);
            connection.httpChunk("");

            connection.request_pending = false;
        }
        for(string& message : connection.websock_pending)
        {
            websocket_handlers[connection.request.path](message);
        }
        connection.websock_pending.clear();
    }
}

Server::Connection::Connection(Server& server)
: server(server)
{
    buffer.reserve(4096);
    state = State::HTTPRequest;
}

bool Server::Connection::update()
{
    char receive_buffer[4096];
    size_t received_size;
    received_size = socket.receive(receive_buffer, sizeof(receive_buffer) - buffer.size());
    if (received_size < 1)
        return false;
    buffer.resize(buffer.size() + received_size);
    memcpy(&buffer[buffer.size()] - received_size, receive_buffer, received_size);
    
    switch(state)
    {
    case State::HTTPRequest:{
        int headers_end = buffer.find("\r\n\r\n");
        if (headers_end)
        {
            std::vector<string> header_data = buffer.substr(0, headers_end).split("\r\n");
            
            std::vector<string> parts = header_data[0].split();
            if (parts.size() != 3)
                return false;
            request.method = parts[0];
            request.path = parts[1];
            request.post_data = "";
            request.headers.clear();
            for(unsigned int n=1; n<header_data.size(); n++)
            {
                std::vector<string> parts = header_data[n].split(":", 1);
                if (parts.size() != 2)
                    return false;
                else
                    request.headers[parts[0].strip().lower()] = parts[1].strip();
            }
            int post_length = 0;
            if (request.headers.find("content-length") != request.headers.end())
            {
                post_length = stringutil::convert::toInt(request.headers["content-length"]);
                if (int(buffer.size()) < headers_end + 4 + post_length)
                    break; //Not enough data yet, continue receiving.
                request.post_data = buffer.substr(headers_end + 4, headers_end + 4 + post_length);
            }
            
            buffer = buffer.substr(headers_end + 4 + post_length);
            handleRequest(request);
        }
        }break;
    case State::Websocket:
        while(true)
        {
            if (buffer.size() < 2)
                return true;
            unsigned int size = buffer[1] & 0x7f;
            int opcode = buffer[0] & 0x0f;
            bool fin = buffer[0] & 0x80;
            bool mask = buffer[1] & 0x80;
            unsigned int index = 2;
                        
            //Close the connection if any of the RSV bits are set.
            if (buffer[0] & 0x70)
            {
                LOG(Warning, "Closing websocket due to RSV bits, we do not support extensions.");
                return false;
            }
            
            if (!fin)
            {
                LOG(Warning, "Websocket frame did not have fin bit set. Implementation does not support fragmentation. So closing connection.");
                return false;
            }
            
            if (size == 126)
            {
                if (buffer.size() < index + 2)
                    return true;
                size = uint8_t(buffer[index++]) << 8;
                size |= uint8_t(buffer[index++]);
            }else if (size == 127)
            {
                if (buffer.size() < index + 8)
                    return true;
                index += 4;
                size = uint8_t(buffer[index++]) << 24;
                size |= uint8_t(buffer[index++]) << 16;
                size |= uint8_t(buffer[index++]) << 8;
                size |= uint8_t(buffer[index++]);
            }
            
            uint8_t mask_values[4] = {0, 0, 0, 0};
            if (mask)
            {
                if (buffer.size() < index + 4)
                    return true;
                for(unsigned int n=0; n<4; n++)
                    mask_values[n] = buffer[index++];
                if (buffer.size() < index + size)
                    return true;
                for(unsigned int n=0; n<size; n++)
                    buffer[index + n] ^= mask_values[n % 4];
            }
            if (buffer.size() < index + size)
                return true;
            
            string message = buffer.substr(index, index + size);
            buffer = buffer.substr(index + size);

            switch(opcode)
            {
            case 0x00://continuation
                LOG(Warning, "Got a websocket continuation frame, this generally only happens on large packets and isn't implemented, closing connection.");
                return false;
            case 0x01://text
                if (server.websocket_handlers.find(request.path) != server.websocket_handlers.end())
                    websock_pending.push_back(message);
                else
                    LOG(Warning, "Websocket text message for unknown path:", request.path);
                break;
            case 0x02://binary
                if (server.websocket_handlers.find(request.path) != server.websocket_handlers.end())
                    websock_pending.push_back(message);
                else
                    LOG(Warning, "Websocket binary message for unknown path:", request.path);
                break;
            case 0x08://close
                {
                    uint8_t reply[] = {0x88, 0};//close packet
                    socket.send(reply, sizeof(reply));
                }
                return false;
            case 0x09://ping
                {
                    uint8_t reply[] = {0x8a, 0};//pong packet
                    socket.send(reply, sizeof(reply));
                }
                break;
            case 0x0a://pong
                LOG(Info, "Websocket pong");
                break;
            }
        }
        break;
    }
    
    return true;
}

void Server::Connection::handleRequest(const Request& request)
{
    if (server.http_handlers.find(request.path) != server.http_handlers.end())
    {
        //Mark this request to be handled in the main thread.
        request_pending = true;
        return;
    }

    //Check if this is a websocket upgrade request.
    //We allow any path to be a websocket.
    if (request.method == "GET"
    && request.headers.find("upgrade") != request.headers.end()
    && request.headers.find("connection") != request.headers.end()
    && request.headers.find("sec-websocket-version") != request.headers.end()
    && request.headers.find("sec-websocket-key") != request.headers.end()
    )
    {
        if (request.headers.find("upgrade")->second.lower() == "websocket"
        && request.headers.find("connection")->second.lower() == "upgrade"
        && request.headers.find("sec-websocket-version")->second.lower() == "13"
        )
        {
            if (server.websocket_handlers.find(request.path) != server.websocket_handlers.end())
            {
                string reply = "HTTP/1.1 101 Switching Protocols\r\n";
                reply += "Upgrade: websocket\r\n";
                reply += "Connection: upgrade\r\n";
                reply += "Sec-WebSocket-Accept: " + stringutil::SHA1(request.headers.find("sec-websocket-key")->second + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").base64() + "\r\n";
                if (request.headers.find("sec-websocket-protocol") != request.headers.end())
                    reply += "Sec-WebSocket-Protocol: chat\r\n";
                reply += "\r\n";
                socket.send(reply.c_str(), reply.size());
                
                state = State::Websocket;
            }
            else
            {
                LOG(Warning, "Tried to open websocket without handler:", request.path);
                startHttpReply(404);
                httpChunk("404 - File not found.");
                httpChunk("");
            }
            return;
        }
    }

    string full_path;
    
    if (request.path.find("..") == -1)
    {
        full_path = server.static_file_path + request.path;
    }
    
    FILE* f = fopen(full_path.c_str(), "rb");
    if (f)
    {
        startHttpReply(200);
        while(true)
        {
            char buffer[1024];
            size_t n = fread(buffer, 1, sizeof(buffer), f);
            if (n < 1)
                break;

            string chunk_len_string = string::hex(n) + "\r\n";
            socket.send(chunk_len_string.c_str(), chunk_len_string.size());
            socket.send(buffer, n);
            socket.send("\r\n", 2);
        }
        fclose(f);
    }
    else
    {
        startHttpReply(404);
        httpChunk("404 - File not found.");

        LOG(Warning, "File not found:", request.path);
    }
    httpChunk("");
}

void Server::Connection::startHttpReply(int reply_code)
{
    string reply = string("HTTP/1.1 ") + string(reply_code) + " OK\r\n";
    reply += "Connection: Keep-Alive\r\n";
    reply += "Transfer-Encoding: chunked\r\n";
    reply += "\r\n";
    socket.send(reply.c_str(), reply.size());
}

void Server::Connection::httpChunk(const string data)
{
    string chunk_len_string = string::hex(data.size()) + "\r\n";
    socket.send(chunk_len_string.c_str(), chunk_len_string.size());
    if (data.size() > 0)
        socket.send(data.data(), data.size());
    socket.send("\r\n", 2);
}

void Server::Connection::sendWebsocketTextPacket(string data)
{
    if (data.size() < 126)
    {
        uint8_t header[] = {
            0x81, //fin + text
            uint8_t(data.size()),
        };
        socket.send(header, sizeof(header));
    }
    else if (data.size() < 0x10000)
    {
        uint8_t header[] = {
            0x81, //fin + text
            126,
            uint8_t((data.size() >> 8) & 0xFF),
            uint8_t(data.size() & 0xFF),
        };
        socket.send(header, sizeof(header));
    }
    else
    {
        uint8_t header[] = {
            0x81, //fin + text
            127,
            0, 0, 0, 0,
            uint8_t((data.size() >> 24) & 0xFF),
            uint8_t((data.size() >> 16) & 0xFF),
            uint8_t((data.size() >> 8) & 0xFF),
            uint8_t(data.size() & 0xFF),
        };
        socket.send(header, sizeof(header));
    }
    
    socket.send(data.data(), data.size());
}

};//namespace http
};//namespace io
};//namespace sp
