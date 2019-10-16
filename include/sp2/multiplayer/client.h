#ifndef SP2_MULTIPLAYER_CLIENT_H
#define SP2_MULTIPLAYER_CLIENT_H

#include <sp2/string.h>
#include <sp2/updatable.h>
#include <sp2/multiplayer/base.h>
#include <sp2/io/dataBuffer.h>
#include <sp2/io/network/tcpSocket.h>
#include <sp2/io/http/websocket.h>


#include <list>

namespace sp {
class Engine;
class Node;
namespace multiplayer {

class Client : public Updatable, public Base
{
public:
    enum State
    {
        Connecting,     //Still connecting to the server, if connecting fails, state will become disconnected.
        Synchronizing,  //Connected to the server, getting data to setup the whole game state.
        Running,        //Connected to the server, game is running and getting updates from the server.
        Disconnected    //Disconnected, server gone missing or never managed to connect at all.
    };

    Client(const string& game_name, uint32_t game_version);
    ~Client();

    // Connect directly to a server with a tcp IP connection.
    bool connect(const string& hostname, int port_nr);
    // Connect to a server by using a switchboard.
    // The hotname and port_nr are the hostname and port of the switchboard server.
    // The given key is the game key given to the server that we want to connect to,
    //      generally entered by the user.
    bool connectBySwitchboard(const string& hostname, int port_nr, const string& key);

    State getState() const { return state; }

    virtual uint32_t getClientId() override;
private:
    io::network::TcpSocket socket;
    io::http::Websocket websocket;
    std::list<io::DataBuffer> send_queue;
    State state = State::Disconnected;
    uint32_t client_id = 0;
    string game_name;
    uint32_t game_version;

    virtual void onUpdate(float delta) override;
    void send(const io::DataBuffer& buffer);
    
    friend class ::sp::Engine;
};

}//namespace multiplayer
}//namespace sp

#endif//SP2_MULTIPLAYER_CLIENT_H
