#ifndef SP2_MULTIPLAYER_CLIENT_H
#define SP2_MULTIPLAYER_CLIENT_H

#include <sp2/string.h>
#include <sp2/updatable.h>

#include <SFML/Network/TcpSocket.hpp>

#include <list>

namespace sp {
class Engine;
namespace multiplayer {

class Client : public Updatable
{
public:
    enum State
    {
        Connecting,     //Still connecting to the server, if connecting fails, state will become disconnected.
        Synchronizing,  //Connected to the server, getting data to setup the whole game state.
        Running,        //Connected to the server, game is running and getting updates from the server.
        Disconnected    //Disconnected, server gone missing or never managed to connect at all.
    };

    Client(string hostname, int port_nr);
    ~Client();
    
    State getState() const { return state; }
    
private:
    sf::TcpSocket socket;
    std::list<sf::Packet> send_queue;
    State state;
    uint32_t client_id;
    
    virtual void onUpdate(float delta) override;
    void send(sf::Packet& packet);
    
    friend class ::sp::Engine;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_CLIENT_H
