#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/registry.h>
#include <private/multiplayer/packetIDs.h>
#include <sp2/scene/scene.h>
#include <sp2/engine.h>
#include <sp2/assert.h>


namespace sp {
namespace multiplayer {

//Until we have C++17, we need to have a decleration of static constexpr values for reference reasons.
constexpr uint8_t PacketIDs::request_authentication;
constexpr uint8_t PacketIDs::set_client_id;
constexpr uint8_t PacketIDs::change_game_speed;
constexpr uint8_t PacketIDs::create_object;
constexpr uint8_t PacketIDs::update_object;
constexpr uint8_t PacketIDs::delete_object;
constexpr uint8_t PacketIDs::setup_scene;
constexpr uint8_t PacketIDs::alive;
constexpr uint64_t PacketIDs::magic_sp2_value;


Server::Server(int port_nr)
{
    if (new_connection_listener.listen(port_nr))
        LOG(Error, "Failed to listen on port: ", port_nr);
    new_connection_listener.setBlocking(false);
    new_connection_socket = new io::network::TcpSocket();
    
    next_client_id = 1;
    next_object_id = 1;
}

Server::~Server()
{
    delete new_connection_socket;
}

void Server::recursiveAddNewNodes(P<Node> node)
{
    if (node->multiplayer.enabled)
    {
        if (node->multiplayer.id == 0)
            addNewObject(node);
        for(P<Node> child : node->getChildren())
            recursiveAddNewNodes(child);
    }
}

void Server::onUpdate(float delta)
{
    for(P<Scene> scene : Scene::all())
    {
        recursiveAddNewNodes(scene->getRoot());
    }

    //When creating new objects, we first send out packets for all objects to be created.
    //And then we send out variable value updates. This because else we could update a pointer variable to an object that does not exist yet.
    for(P<Node> node : new_nodes)
    {
        io::DataBuffer packet;
        buildCreatePacket(packet, node);
        
        sendToAllConnectedClients(packet);
        
        addNode(node);
    }
    for(P<Node> node : new_nodes)
    {
        if (node->multiplayer.replication_links.size() > 0)
        {
            io::DataBuffer packet(PacketIDs::update_object, node->multiplayer.getId());
            for(unsigned int n=0; n<node->multiplayer.replication_links.size(); n++)
            {
                ReplicationLinkBase* replication_link = node->multiplayer.replication_links[n];
                packet.write(uint16_t(n));
                replication_link->initialSend(*this, packet);
            }
            sendToAllConnectedClients(packet);
        }
    }
    new_nodes.clear();
    
    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
    {
        io::DataBuffer packet;
        packet.write(PacketIDs::update_object, it->second->multiplayer.getId());
        unsigned int zero_data_size = packet.getDataSize();
        for(unsigned int n=0; n<it->second->multiplayer.replication_links.size(); n++)
        {
            ReplicationLinkBase* replication_link = it->second->multiplayer.replication_links[n];
            if (replication_link->isChanged())
            {
                packet.write(uint16_t(n));
                replication_link->send(*this, packet);
            }
        }
        if (packet.getDataSize() != zero_data_size)
            sendToAllConnectedClients(packet);
    }
    
    //Check for new connections.
    if (new_connection_listener.accept(*new_connection_socket))
    {
        LOG(Info, "Accepted new connection on server");
        new_connection_socket->setBlocking(false);
        
        ClientInfo client;
        client.socket = new_connection_socket;
        client.client_id = next_client_id;
        next_client_id ++;
        client.state = ClientInfo::State::WaitingForAuthentication;
        io::DataBuffer packet(PacketIDs::request_authentication, PacketIDs::magic_sp2_value);
        client.socket->send(packet);
        clients.push_back(client);

        new_connection_socket = new io::network::TcpSocket();
    }
    
    for(auto client = clients.begin(); client != clients.end(); )
    {
        io::DataBuffer packet;
        while(client->socket->receive(packet))
        {
            uint8_t packet_id;
            packet.read(packet_id);
            switch(client->state)
            {
            case ClientInfo::State::WaitingForAuthentication:
                switch(packet_id)
                {
                case PacketIDs::request_authentication:
                    {
                        io::DataBuffer send_packet(PacketIDs::set_client_id, client->client_id);
                        client->send(send_packet);
                    }
                    {
                        io::DataBuffer send_packet(PacketIDs::change_game_speed, Engine::getInstance()->getGameSpeed());
                        client->send(send_packet);
                    }

                    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
                    {
                        io::DataBuffer send_packet;
                        buildCreatePacket(send_packet, *it->second);
                        client->send(send_packet);
                    }
                    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
                    {
                        if (it->second->multiplayer.replication_links.size() > 0)
                        {
                            io::DataBuffer send_packet(PacketIDs::update_object, it->first);
                            for(unsigned int n=0; n<it->second->multiplayer.replication_links.size(); n++)
                            {
                                ReplicationLinkBase* replication_link = it->second->multiplayer.replication_links[n];
                                send_packet.write(uint16_t(n));
                                replication_link->send(*this, send_packet);
                            }
                            client->send(send_packet);
                        }
                    }
                    client->state = ClientInfo::State::Connected;
                    break;
                default:
                    LOG(Warning, "Unknown packet during authentication, id:", packet_id);
                }
                break;
            case ClientInfo::State::CatchingUp:
            case ClientInfo::State::Connected:
                break;
            }
        }
        if (!client->socket->isConnected())
        {
            client = clients.erase(client);
        }
        else
        {
            client++;
        }
    }
}

void Server::buildCreatePacket(io::DataBuffer& packet, P<Node> node)
{
    auto e = multiplayer::ClassEntry::type_to_name_mapping.find(typeid(*node));
    sp2assert(e != multiplayer::ClassEntry::type_to_name_mapping.end(), (string("No multiplayer class registry for ") + typeid(*node).name()).c_str());
    
    P<Node> parent = node->getParent();

    if (parent)
    {
        packet.write(PacketIDs::create_object, node->multiplayer.getId(), e->second, parent->multiplayer.getId());
    }
    else
    {
        //This node is the root node of a scene, so setup our scene information.
        P<Scene> scene = node->getScene();
        packet.write(PacketIDs::setup_scene, node->multiplayer.getId(), scene->getName());
    }
}

void Server::onDeleted(uint64_t id)
{
    sendToAllConnectedClients(io::DataBuffer(PacketIDs::delete_object, id));
}

void Server::addNewObject(P<Node> node)
{
    node->multiplayer.id = next_object_id;
    next_object_id++;
    new_nodes.add(node);
}

void Server::sendToAllConnectedClients(const io::DataBuffer& packet)
{
    for(auto client : clients)
    {
        if (client.state != ClientInfo::State::Connected)
            continue;
        client.send(packet);
    }
}

};//namespace multiplayer
};//namespace sp
