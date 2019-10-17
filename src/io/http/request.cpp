#include <sp2/io/http/request.h>
#include <sp2/stringutil/convert.h>

namespace sp {
namespace io {
namespace http {

Request::Request(const string& host, int port, Scheme scheme)
: port(port), scheme(scheme)
{
    headers["Host"] = host;
}

void Request::setHeader(const string& key, const string& value)
{
    headers[key] = value;
    if (key == "Host")
        socket.close();
}

Request::Response Request::get(const string& path)
{
    return request("GET", path, "");
}

Request::Response Request::post(const string& path, const string& data)
{
    return request("POST", path, data);
}

Request::Response Request::request(const string& method, const string& path, const string& data)
{
    if (!socket.isConnected())
    {
        if (scheme == Scheme::Auto)
            scheme = ((port == 443) ? Scheme::Https : Scheme::Http);
        if (scheme == Scheme::Http)
            socket.connect(io::network::Address(headers["Host"]), port);
        else
            socket.connectSSL(io::network::Address(headers["Host"]), port);
        socket.setTimeout(5000);
    }
    Response response;

    string request = method + " " + path + " HTTP/1.1\r\n";
    for(auto& h : headers)
        request += h.first + ": " + h.second + "\r\n";
    if (data.length() > 0)
        request += "Content-Length: " + string(int(data.length())) + "\r\n";
    request += "\r\n";

    socket.send(request.data(), request.length());
    if (data.length() > 0)
        socket.send(data.data(), data.length());

    char receive_buffer[4096];
    int received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
    string received_data(receive_buffer, received_size);
    while(received_data.find("\r\n\r\n") < 0)
    {
        received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
        received_data += string(receive_buffer, received_size);
        if (received_size == 0)
            return response;
    }

    std::vector<string> response_line = received_data.substr(0, received_data.find("\r\n")).split(" ");
    received_data = received_data.substr(received_data.find("\r\n") + 2);

    for(auto& header_line : received_data.substr(0, received_data.find("\r\n\r\n")).split("\r\n"))
    {
        int idx = header_line.find(":");
        if (idx > -1)
            response.headers[header_line.substr(0, idx).strip()] = header_line.substr(idx + 1).strip();
    }
    received_data = received_data.substr(received_data.find("\r\n\r\n") + 4);

    if (response_line.size() > 1)
        response.status = stringutil::convert::toInt(response_line[1]);

    if (response.headers.find("Content-Length") != response.headers.end())
    {
        int content_length = stringutil::convert::toInt(response.headers["Content-Length"]);
        response.body = std::move(received_data);
        while(int(response.body.length()) < content_length)
        {
            received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
            response.body += string(receive_buffer, received_size);
            if (received_size == 0)
                return response;
        }
    }
    else if (response.headers.find("Transfer-Encoding") != response.headers.end() && response.headers["Transfer-Encoding"] == "chunked")
    {
        int chunk_size;
        do
        {
            while(received_data.find("\r\n") < 0)
            {
                received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
                received_data += string(receive_buffer, received_size);
                if (received_size == 0)
                    return response;
            }
            chunk_size = stringutil::convert::toInt(received_data.substr(0, received_data.find("\r\n")), 16);
            received_data = received_data.substr(received_data.find("\r\n") + 2);
            while(int(received_data.length()) < chunk_size + 2)
            {
                received_size = socket.receive(receive_buffer, sizeof(receive_buffer));
                received_data += string(receive_buffer, received_size);
                if (received_size == 0)
                    return response;
            }
            response.body += received_data.substr(0, chunk_size);
            received_data = received_data.substr(chunk_size + 2);
        } while(chunk_size > 0);
    }

    response.success = true;
    return response;
}

}//namespace http
}//namespace io
}//namespace sp
