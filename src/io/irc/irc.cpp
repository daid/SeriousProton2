#include <sp2/io/irc/irc.h>

namespace sp {
namespace io {

Irc::Irc(const string& server, const string& user, const string& nick, const string& password, int port)
{
    if (socket.connect(network::Address(server), port))
    {
        socket.setBlocking(false);

        string connect_request = "PASS " + password + "\r\nUSER " + user + "\r\nNICK " + nick + "\r\n";
        socket.send(connect_request.data(), connect_request.length());
    }
}

bool Irc::isConnected()
{
    return socket.isConnected();
}

void Irc::onUpdate(float delta)
{
    char buffer[128];
    size_t received_size = socket.receive(buffer, sizeof(buffer));
    if (received_size > 0)
    {
        received_data += string(buffer, received_size);
        while(received_data.find_first_of("\r\n") != std::string::npos)
        {
            size_t eol = received_data.find_first_of("\r\n");
            if (eol > 0)
            {
                processLine(received_data.substr(0, eol));
            }
            received_data = received_data.substr(eol + 1);
        }
    }
}

void Irc::processLine(string line)
{
    string prefix;
    int idx;
    //TODO: IRCv3 has "@tags" in front of the line, before the ":"
    if (line[0] == ':')
    {
        idx = line.find(" ");
        prefix = line.substr(1, idx);
        line = line.substr(idx + 1);
    }
    idx = line.find(" ");
    string command = line.substr(0, idx);
    line = line.substr(idx + 1);
    idx = line.find(":");
    string trailing;
    if (idx > -1)
    {
        trailing = line.substr(idx + 1);
        line = line.substr(0, idx).strip();
    }

    processLine(prefix, command, line, trailing);
}

void Irc::processLine(string prefix, string command, string params, string trailing)
{
    LOG(Debug, "IRC: <", prefix, "#", command, "#", params, "#", trailing);
    if (command == "001")
    {
        onConnected(trailing);
    }
    else if (command == "PING")
    {
        string pong = "PONG :" + trailing + "\r\n";
        socket.send(pong.data(), pong.length());
    }
    else if (command == "PRIVMSG")
    {
        if (prefix.find("!") != -1)
            prefix = prefix.substr(0, prefix.find("!"));
        if (params.startswith("#"))
            onMessage(params.substr(1), prefix, trailing);
    }
}

void Irc::sendRaw(const string& data)
{
    socket.send(data.data(), data.length());
    socket.send("\r\n", 2);
}

void Irc::joinChannel(const string& channel)
{
    string command = "JOIN #" + channel + "\r\n";
    socket.send(command.data(), command.length());
}

void Irc::sendMessage(const string& channel, const string& message)
{
    string command = "PRIVMSG #" + channel + " :" + message + "\r\n";
    socket.send(command.data(), command.length());
}

void Irc::onConnected(const string& welcome_message)
{
    LOG(Info, "IRC Connected:", welcome_message);
}

void Irc::onMessage(const string& channel, const string& user, const string& message)
{
    LOG(Info, "IRC Message:", channel, user, message);
}

}//namespace io
}//namespace sp
