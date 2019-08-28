#ifndef SP2_MULTIPLAYER_CLIENT_H
#define SP2_MULTIPLAYER_CLIENT_H

#include <sp2/string.h>
#include <sp2/updatable.h>
#include <sp2/multiplayer/base.h>
#include <sp2/io/dataBuffer.h>
#include <sp2/io/network/tcpSocket.h>


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

    Client(uint32_t game_id, uint32_t game_version, string hostname, int port_nr);
    ~Client();
    
    State getState() const { return state; }
    
    virtual uint32_t getClientId() override;
private:
    io::network::TcpSocket socket;
    std::list<io::DataBuffer> send_queue;
    State state;
    uint32_t client_id;
    uint32_t game_id;
    uint32_t game_version;

    virtual void onUpdate(float delta) override;
    void send(const io::DataBuffer& buffer);
    
    friend class ::sp::Engine;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_CLIENT_H
