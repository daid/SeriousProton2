#ifndef SP2_MULTIPLAYER_SERVER_H
#define SP2_MULTIPLAYER_SERVER_H

#include <sp2/scene/node.h>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpListener.hpp>

#include <list>
#include <unordered_map>

namespace sp {
class Engine;
namespace multiplayer {

class Server : public AutoPointerObject
{
public:
    Server(int port_nr);
    ~Server();
private:
    class ClientInfo
    {
    public:
        sf::TcpSocket* socket;
        uint32_t client_id;
        enum class State
        {
            WaitingForAuthentication,
            CatchingUp,
            Connected
        } state;
        std::list<sf::Packet> send_queue;
    };

    uint32_t next_client_id;
    //Next id for a new object.
    uint64_t next_object_id;
    //List of newly created objects.
    PList<SceneNode> new_nodes;
    
    std::unordered_map<uint64_t, P<SceneNode>> node_by_id;
    std::list<ClientInfo> clients;
    
    sf::TcpListener new_connection_listener;
    sf::TcpSocket* new_connection_socket;
    
    void recursiveAddInitial(SceneNode* node);
    //Add a new object to be replicated. Only put it in a list, we will process it later, as it still might be under construction.
    void addNewObject(SceneNode* node);
    
    void update();
    
    void sendToAllConnectedClients(sf::Packet& packet);

    friend class SceneNode::Multiplayer;
    friend class ::sp::Engine;
public:
    static Server* getInstance() { return instance; }
private:
    static Server* instance;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_SERVER_H
