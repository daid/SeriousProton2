#ifndef SP2_MULTIPLAYER_CLIENT_H
#define SP2_MULTIPLAYER_CLIENT_H

#include <sp2/string.h>
#include <sp2/pointer.h>

namespace sp {
class Engine;
namespace multiplayer {

class Client : public AutoPointerObject
{
public:
    enum State
    {
        Connecting,     //Still connecting to the server, if connecting fails, state will become disconnected.
        Synchronizing,  //Connected to the server, getting data to setup the whole game state.
        Running,        //Connected to the server, game is running and getting updates from the server.
        Disconnected    //Disconnected, server gone missing or never managed to connect at all.
    };

    Client(string hostname);
    ~Client();
    
    State getState() const { return state; }
    
private:
    State state;
    uint32_t client_id;
    
    void update();
    
    friend class ::sp::Engine;
public:
    static Client* getInstance() { return instance; }
private:
    static Client* instance;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_CLIENT_H
