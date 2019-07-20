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
constexpr uint8_t PacketIDs::call_on_server;
constexpr uint8_t PacketIDs::alive;
constexpr uint64_t PacketIDs::magic_sp2_value;


Server::Server(int port_nr)
{
    if (!new_connection_listener.listen(port_nr))
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

void Server::recursiveSendCreate(ClientInfo& client, P<Node> node)
{
    if (node->multiplayer.enabled)
    {
        io::DataBuffer send_packet;
        buildCreatePacket(send_packet, node);
        client.send(send_packet);

        for(P<Node> child : node->getChildren())
            recursiveSendCreate(client, child);
    }
}

void Server::onUpdate(float delta)
{
    std::chrono::duration<float> now = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now().time_since_epoch());

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
            if (replication_link->isChanged(delta))
            {
                packet.write(uint16_t(n));
                replication_link->send(*this, packet);
            }
        }
        if (packet.getDataSize() != zero_data_size)
            sendToAllConnectedClients(packet);

        for(auto& prepared_call : it->second->multiplayer.prepared_calls)
        {
            uint16_t index;
            prepared_call.read(index);
            it->second->multiplayer.replication_calls[index]->doCall(*it->second, prepared_call);
        }
        it->second->multiplayer.prepared_calls.clear();
    }
    
    //Check for new connections.
    if (new_connection_listener.accept(*new_connection_socket))
    {
        LOG(Info, "Accepted new connection on server");
        new_connection_socket->setBlocking(false);
        
        ClientInfo client;
        client.socket = new_connection_socket;
        client.client_id = next_client_id;
        client.current_ping_delay = 0.0;
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

                    for(P<Scene> scene : Scene::all())
                    {
                        recursiveSendCreate(*client, scene->getRoot());
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
                switch(packet_id)
                {
                case PacketIDs::call_on_server:
                    {
                        uint64_t object_id;
                        uint16_t index;
                        packet.read(object_id, index);
                        P<Node> node = getNode(object_id);
                        if (node && index < node->multiplayer.replication_calls.size())   //Node could have been deleted on the server already.
                            node->multiplayer.replication_calls[index]->doCall(*node, packet);
                    }
                    break;
                case PacketIDs::alive:
                    {
                        float request_time;
                        packet.read(request_time);
                        client->current_ping_delay = now.count() - request_time;
                    }
                    break;
                default:
                    LOG(Warning, "Unknown packet from client, id:", packet_id);
                }
                break;
            }
        }
        if (!client->socket->isConnected())
        {
            LOG(Info, "Client connection closed on server");
            client = clients.erase(client);
        }
        else
        {
            client++;
        }
    }
    
    ping_delay -= delta;
    if (ping_delay < 0.0)
    {
        ping_delay += 1.0;
        for(auto client : clients)
        {
            io::DataBuffer ping_packet;
            ping_packet.write(PacketIDs::alive, client.current_ping_delay, now.count());
            client.socket->send(ping_packet);
        }
    }
}

void Server::buildCreatePacket(io::DataBuffer& packet, P<Node> node)
{
    P<Node> parent = node->getParent();

    if (parent)
    {
        auto e = multiplayer::ClassEntry::type_to_name_mapping.find(typeid(**node));
        sp2assert(e != multiplayer::ClassEntry::type_to_name_mapping.end(), (string("No multiplayer class registry for ") + typeid(**node).name()).c_str());

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
