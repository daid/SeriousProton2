#ifndef SP2_MULTIPLAYER_SERVER_H
#define SP2_MULTIPLAYER_SERVER_H

#include <sp2/updatable.h>
#include <sp2/scene/node.h>

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>

#include <list>
#include <unordered_map>

namespace sp {
namespace multiplayer {

class Server : public Updatable
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
        
        void send(sf::Packet& packet)
        {
            if (send_queue.begin() == send_queue.end())
            {
                if (socket->send(packet) == sf::Socket::Partial)
                    send_queue.push_back(packet);
            }
            else
            {
                send_queue.push_back(packet);
            }
        }
    };

    uint32_t next_client_id;
    //Next id for a new object.
    uint64_t next_object_id;
    //List of newly created objects.
    PList<Node> new_nodes;
    
    std::unordered_map<uint64_t, P<Node>> node_by_id;
    std::list<ClientInfo> clients;
    
    sf::TcpListener new_connection_listener;
    sf::TcpSocket* new_connection_socket;
    
    void recursiveAddInitial(Node* node);
    //Add a new object to be replicated. Only put it in a list, we will process it later, as it still might be under construction.
    void addNewObject(Node* node);
    
    virtual void onUpdate(float delta) override;
    
    void buildCreatePacket(sf::Packet& packet, Node* node);
    void sendToAllConnectedClients(sf::Packet& packet);

    friend class Node::Multiplayer;
public:
    static Server* getInstance() { return instance; }
private:
    static Server* instance;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_SERVER_H
