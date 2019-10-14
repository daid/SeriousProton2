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

class Server : public Updatable, public Base
{
public:
    Server(uint32_t game_id, uint32_t game_version, int port_nr);
    ~Server();

    virtual uint32_t getClientId() override;
private:
    class ClientInfo
    {
    public:
        io::network::TcpSocket socket;
        uint32_t client_id;
        float current_ping_delay;
        enum class State
        {
            WaitingForAuthentication,
            CatchingUp,
            Connected
        } state;
        
        void send(const io::DataBuffer& packet)
        {
            socket.send(packet);
        }
    };
    uint32_t game_id;
    uint32_t game_version;

    uint32_t next_client_id;
    //Next id for a new object.
    uint64_t next_object_id;
    //List of newly created objects.
    PList<Node> new_nodes;
    
    float ping_delay;

    std::list<ClientInfo> clients;
    
    io::network::TcpListener new_connection_listener;
    
    void recursiveAddNewNodes(P<Node> node);
    void recursiveSendCreate(ClientInfo& client, P<Node> node);
    //Add a new object to be replicated. Only put it in a list, we will process it later, as it still might be under construction.
    void addNewObject(P<Node> node);
    
    virtual void onUpdate(float delta) override;
    virtual void onDeleted(uint64_t id) override;
    
    void buildCreatePacket(io::DataBuffer& packet, P<Node> node);
    void sendToAllConnectedClients(const io::DataBuffer& packet);

    friend class Node::Multiplayer;
};

}//namespace multiplayer
}//namespace sp

#endif//SP2_MULTIPLAYER_SERVER_H
