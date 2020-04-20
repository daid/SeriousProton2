#ifndef SP2_IO_IRC_IRC_H
#define SP2_IO_IRC_IRC_H

#include <sp2/io/network/tcpSocket.h>
#include <sp2/updatable.h>

namespace sp {
namespace io {

class Irc : public Updatable
{
public:
    Irc(const string& server, const string& user, const string& nick, const string& password, int port=6667);

    bool isConnected();

    virtual void onUpdate(float delta) override;

    virtual void onConnected(const string& welcome_message);
    virtual void onMessage(const string& channel, const string& user, const string& message);

    void joinChannel(const string& channel);
    void sendMessage(const string& channel, const string& message);
protected:
    virtual void processLine(string prefix, string command, string params, string trailing);

    void sendRaw(const string& data);
private:
    void processLine(string line);

    network::TcpSocket socket;
    string received_data;
};

}//namespace io
}//namespace sp


#endif//SP2_IO_IRC_IRC_H
