#ifndef SP2_MULTIPLAYER_SERVER_H
#define SP2_MULTIPLAYER_SERVER_H

#include <sp2/updatable.h>
#include <sp2/scene/node.h>

#include <sp2/io/dataBuffer.h>
#include <sp2/io/network/tcpListener.h>
#include <sp2/io/network/tcpSocket.h>

#include <list>
#include <unordered_map>

namespace sp {
namespace multiplayer {

class Server : public Updatable, public NodeRegistry
{
public:
    Server(int port_nr);
    ~Server();
private:
    class ClientInfo
    {
    public:
        io::network::TcpSocket* socket;
        uint32_t client_id;
        enum class State
        {
            WaitingForAuthentication,
            CatchingUp,
            Connected
        } state;
        
        void send(const io::DataBuffer& packet)
        {
            socket->send(packet);
        }
    };

    uint32_t next_client_id;
    //Next id for a new object.
    uint64_t next_object_id;
    //List of newly created objects.
    PList<Node> new_nodes;

    std::list<ClientInfo> clients;
    
    io::network::TcpListener new_connection_listener;
    io::network::TcpSocket* new_connection_socket;
    
    void recursiveAddNewNodes(Node* node);
    //Add a new object to be replicated. Only put it in a list, we will process it later, as it still might be under construction.
    void addNewObject(Node* node);
    
    virtual void onUpdate(float delta) override;
    virtual void onDeleted(uint64_t id) override;
    
    void buildCreatePacket(io::DataBuffer& packet, Node* node);
    void sendToAllConnectedClients(const io::DataBuffer& packet);

    friend class Node::Multiplayer;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_SERVER_H
