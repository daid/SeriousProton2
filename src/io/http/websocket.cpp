#include <sp2/io/http/websocket.h>
#include <sp2/stringutil/convert.h>
#include <sp2/stringutil/base64.h>
#include <sp2/stringutil/sha1.h>
#include <sp2/random.h>
#include <sp2/logging.h>


namespace sp {
namespace io {
namespace http {
namespace websocket {
    static constexpr int fin_mask = 0x80;
    static constexpr int rsv_mask = 0x70;
    static constexpr int opcode_mask = 0x0f;
    
    static constexpr int mask_mask = 0x80;
    static constexpr int payload_length_mask = 0x7f;
    static constexpr int payload_length_16bit = 126;
    static constexpr int payload_length_64bit = 127;
    
    static constexpr int opcode_continuation = 0x00;
    static constexpr int opcode_text = 0x01;
    static constexpr int opcode_binary = 0x02;
    static constexpr int opcode_close = 0x08;
    static constexpr int opcode_ping = 0x09;
    static constexpr int opcode_pong = 0x0a;
};

Websocket::Websocket()
{
}

Websocket::Websocket(Websocket&& other)
{
    *this = std::move(other);
}

Websocket::~Websocket()
{
}

Websocket& Websocket::operator=(Websocket&& other)
{
    state = other.state;
    websock_key = std::move(other.websock_key);
    socket = std::move(other.socket);
    buffer = std::move(other.buffer);
    received_fragment = std::move(other.received_fragment);
    return *this;
}

bool Websocket::connect(const string& url)
{
    state = State::Disconnected;

    int scheme_length = 5;
    if (!url.startswith("ws://") && !url.startswith("wss://"))
        return false;
    if (url.startswith("wss://"))
        scheme_length = 6;
    int end_of_hostname = url.find("/", scheme_length);
    string hostname = url.substr(scheme_length, end_of_hostname);
    int port = 80;
    if (hostname.find(":") != -1)
    {
        port = sp::stringutil::convert::toInt(hostname.substr(hostname.find(":") + 1));
        hostname = hostname.substr(0, hostname.find(":"));
    }
    if (url.startswith("wss://"))
    {
        if (!socket.connectSSL(sp::io::network::Address(hostname), port))
            return false;
    }
    else
    {
        if (!socket.connect(sp::io::network::Address(hostname), port))
            return false;
    }
    string path = url.substr(end_of_hostname);
    for(int n=0;n<16;n++)
        websock_key += char(sp::irandom(0, 255));
    websock_key = sp::stringutil::base64::encode(websock_key);
    string request = "GET " + path + " HTTP/1.1\r\n"
        "Host: " + hostname + "\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "Sec-Websocket-Version: 13\r\n"
        "Sec-Websocket-Key: " + websock_key + "\r\n"
        "Sec-WebSocket-Protocol: chat\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache, no-store, must-revalidate\r\n"
        "\r\n";
    socket.send(request.data(), request.length());
    state = State::Connecting;
    socket.setBlocking(false);
    return true;
}

void Websocket::close()
{
    socket.close();
    state = State::Disconnected;
}

bool Websocket::isConnected()
{
    return state == State::Operational && socket.isConnected();
}

bool Websocket::isConnecting()
{
    return state == State::Connecting && socket.isConnected();
}

void Websocket::send(const io::DataBuffer& data_buffer)
{
    if (state != State::Operational)
        return;

    if (data_buffer.getDataSize() < websocket::payload_length_16bit)
    {
        uint8_t header[] = {
            websocket::fin_mask | websocket::opcode_text,
            uint8_t(data_buffer.getDataSize()),
        };
        socket.send(header, sizeof(header));
    }
    else if (data_buffer.getDataSize() < (1 << 16))
    {
        uint8_t header[] = {
            websocket::fin_mask | websocket::opcode_text,
            websocket::payload_length_16bit,
            uint8_t((data_buffer.getDataSize() >> 8) & 0xFF),
            uint8_t(data_buffer.getDataSize() & 0xFF),
        };
        socket.send(header, sizeof(header));
    }
    else
    {
        uint8_t header[] = {
            websocket::fin_mask | websocket::opcode_text,
            websocket::payload_length_64bit,
            0, 0, 0, 0,
            uint8_t((data_buffer.getDataSize() >> 24) & 0xFF),
            uint8_t((data_buffer.getDataSize() >> 16) & 0xFF),
            uint8_t((data_buffer.getDataSize() >> 8) & 0xFF),
            uint8_t(data_buffer.getDataSize() & 0xFF),
        };
        socket.send(header, sizeof(header));
    }

    socket.send(data_buffer.getData(), data_buffer.getDataSize());
}

bool Websocket::receive(io::DataBuffer& data_buffer)
{
    char receive_buffer[4096];
    size_t received_size;
    received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
    if (received_size > 0)
    {
        buffer.resize(buffer.size() + received_size);
        memcpy(&buffer[buffer.size()] - received_size, receive_buffer, received_size);
    }

    switch(state)
    {
    case State::Disconnected:
        break;
    case State::Connecting:{
        string buffer_str = string((const char*)buffer.data(), buffer.size());
        int headers_end = buffer_str.find("\r\n\r\n");
        if (headers_end)
        {
            std::vector<string> header_data = buffer_str.substr(0, headers_end).split("\r\n");
            buffer.erase(buffer.begin(), buffer.begin() + headers_end + 4);
            std::vector<string> parts = header_data[0].split(" ", 2);
            if (parts.size() != 3)
            {
                LOG(Warning, "Connecting to websocket failed, incorrect reply on HTTP upgrade request");
                close();
                return false;
            }
            if (parts[1] != "101")
            {
                LOG(Warning, "Connecting to websocket failed, incorrect reply on HTTP upgrade request");
                close();
                return false;
            }

            std::unordered_map<string, string> headers;
            for(unsigned int n=1; n<header_data.size(); n++)
            {
                std::vector<string> parts = header_data[n].split(":", 1);
                if (parts.size() == 2)
                    headers[parts[0].strip().lower()] = parts[1].strip();
            }
            if (headers.find("upgrade") == headers.end() || headers.find("connection") == headers.end() || headers.find("sec-websocket-accept") == headers.end())
            {
                LOG(Warning, "Connecting to websocket failed, incorrect reply on HTTP upgrade request");
                close();
                return false;
            }
            if (headers["upgrade"].lower() != "websocket" || headers["connection"].lower() != "upgrade")
            {
                LOG(Warning, "Connecting to websocket failed, incorrect reply on HTTP upgrade request");
                close();
                return false;
            }
            if (headers["sec-websocket-accept"] != stringutil::SHA1(websock_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").base64())
            {
                LOG(Warning, "Connecting to websocket failed, incorrect reply on HTTP upgrade request");
                close();
                return false;
            }
            state = State::Operational;
        }
        }break;
    case State::Operational:
        while(true)
        {
            if (buffer.size() < 2)
                return false;
            unsigned int payload_length = buffer[1] & websocket::payload_length_mask;
            int opcode = buffer[0] & websocket::opcode_mask;
            bool fin = buffer[0] & websocket::fin_mask;
            bool mask = buffer[1] & websocket::mask_mask;
            unsigned int index = 2;

            //Close the connection if any of the RSV bits are set.
            if (buffer[0] & websocket::rsv_mask)
            {
                LOG(Warning, "Closing client websocket due to RSV bits, we do not support extensions.");
                close();
                return false;
            }

            if (payload_length == websocket::payload_length_16bit)
            {
                if (buffer.size() < index + 2)
                    return false;
                payload_length = uint8_t(buffer[index++]) << 8;
                payload_length |= uint8_t(buffer[index++]);
            }else if (payload_length == websocket::payload_length_64bit)
            {
                if (buffer.size() < index + 8)
                    return false;
                index += 4;
                payload_length = uint8_t(buffer[index++]) << 24;
                payload_length |= uint8_t(buffer[index++]) << 16;
                payload_length |= uint8_t(buffer[index++]) << 8;
                payload_length |= uint8_t(buffer[index++]);
            }

            uint8_t mask_values[4] = {0, 0, 0, 0};
            if (mask)
            {
                if (buffer.size() < index + 4)
                    return false;
                for(unsigned int n=0; n<4; n++)
                    mask_values[n] = buffer[index++];
                if (buffer.size() < index + payload_length)
                    return false;
                for(unsigned int n=0; n<payload_length; n++)
                    buffer[index + n] ^= mask_values[n % 4];
            }
            if (buffer.size() < index + payload_length)
                return false;

            std::vector<uint8_t> message(buffer.begin() + index, buffer.begin() + index + payload_length);
            buffer.erase(buffer.begin(), buffer.begin() + index + payload_length);

            switch(opcode)
            {
            case websocket::opcode_continuation:
                received_fragment.insert(received_fragment.end(), message.begin(), message.end());
                if (fin)
                {
                    data_buffer = std::move(received_fragment);
                    received_fragment.clear();
                    return true;
                }
                break;
            case websocket::opcode_text:
            case websocket::opcode_binary:
                if (fin)
                {
                    data_buffer = std::move(message);
                    return true;
                }
                else
                {
                    received_fragment = message;
                }
                break;
            case websocket::opcode_close:
                {
                    uint8_t reply[] = {websocket::fin_mask | websocket::opcode_close, 0};//close packet
                    socket.send(reply, sizeof(reply));
                }
                close();
                return false;
            case websocket::opcode_ping:
                {
                    //Note: The standard says that we need to include the payload of the ping packet as payload in the pong packet.
                    //      We ignore this, as this no client seems to use this.
                    uint8_t reply[] = {websocket::fin_mask | websocket::opcode_pong, 0};//pong packet
                    socket.send(reply, sizeof(reply));
                }
                break;
            case websocket::opcode_pong:
                //There is no real need to track PONG replies. TCP/IP will close the connection if the other side is gone.
                break;
            }
        }
    }
    return false;
}

void Websocket::send(const string& message)
{
    io::DataBuffer data_buffer;
    data_buffer.appendRaw(message.data(), message.size());
    send(data_buffer);
}

bool Websocket::receive(string& output)
{
    io::DataBuffer data_buffer;
    if (!receive(data_buffer))
        return false;
    output = string((const char*)data_buffer.getData(), data_buffer.getDataSize());
    return true;
}

}//namespace http
}//namespace io
}//namespace sp
