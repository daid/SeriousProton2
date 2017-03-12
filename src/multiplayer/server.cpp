#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/registry.h>
#include <private/multiplayer/packetIDs.h>
#include <sp2/scene/scene.h>
#include <sp2/engine.h>
#include <sp2/assert.h>

#include <SFML/Network/TcpSocket.hpp>

namespace sp {
namespace multiplayer {

Server* Server::instance;

Server::Server(int port_nr)
{
    sp2assert(!instance, "Only a single multiplayer::Server instance can exists");
    instance = this;

    if (new_connection_listener.listen(port_nr) != sf::Socket::Done)
        LOG(Error, "Failed to listen on port: ", port_nr);
    new_connection_listener.setBlocking(false);
    new_connection_socket = new sf::TcpSocket();
    
    next_client_id = 1;
    next_object_id = 1;
    
    for(Scene* scene : Scene::scenes)
    {
        recursiveAddInitial(*scene->getRoot());
    }
}

Server::~Server()
{
    delete new_connection_socket;
    
    instance = nullptr;
}

void Server::recursiveAddInitial(SceneNode* node)
{
    if (node->multiplayer.enable_replication)
    {
        addNewObject(node);
        for(SceneNode* child : node->getChildren())
        {
            recursiveAddInitial(child);
        }
    }
}

void Server::update()
{
    //When creating new objects, we first send out packets for all objects to be created.
    //And then we send out variable value updates. This because else we could update a pointer variable to an object that does not exist yet.
    for(SceneNode* node : new_nodes)
    {
        sf::Packet packet;
        buildCreatePacket(packet, node);
        
        sendToAllConnectedClients(packet);
        
        node_by_id[node->multiplayer.getId()] = node;
    }
    for(SceneNode* node : new_nodes)
    {
        if (node->multiplayer.replication_links.size() > 0)
        {
            sf::Packet packet;
            packet << PacketIDs::update_object << node->multiplayer.getId();
            for(unsigned int n=0; n<node->multiplayer.replication_links.size(); n++)
            {
                ReplicationLinkBase* replication_link = node->multiplayer.replication_links[n];
                replication_link->isChanged();
                packet << uint16_t(n);
                replication_link->handleSend(packet);
            }
            sendToAllConnectedClients(packet);
        }
    }
    new_nodes.clear();
    
    std::vector<uint64_t> delete_list;
    for(auto it : node_by_id)
    {
        if (it.second)
        {
            sf::Packet packet;
            packet << PacketIDs::update_object << it.second->multiplayer.getId();
            unsigned int zero_data_size = packet.getDataSize();
            for(unsigned int n=0; n<it.second->multiplayer.replication_links.size(); n++)
            {
                ReplicationLinkBase* replication_link = it.second->multiplayer.replication_links[n];
                if (replication_link->isChanged())
                {
                    packet << uint16_t(n);
                    replication_link->handleSend(packet);
                }
            }
            if (packet.getDataSize() != zero_data_size)
                sendToAllConnectedClients(packet);
        }
        else
        {
            delete_list.push_back(it.first);
        }
    }
    for(auto id : delete_list)
    {
        sf::Packet packet;
        packet << PacketIDs::delete_object << id;
        sendToAllConnectedClients(packet);
        node_by_id.erase(id);
    }
    
    //Check for new connections.
    if (new_connection_listener.accept(*new_connection_socket) == sf::Socket::Done)
    {
        LOG(Info, "Accepted new connection on server");
        new_connection_socket->setBlocking(false);
        
        ClientInfo client;
        client.socket = new_connection_socket;
        client.client_id = next_client_id;
        next_client_id ++;
        client.state = ClientInfo::State::WaitingForAuthentication;
        sf::Packet packet;
        packet << PacketIDs::request_authentication << PacketIDs::magic_sp2_value;
        if (client.socket->send(packet) == sf::Socket::Partial)
            client.send_queue.push_back(packet);
        clients.push_back(client);

        new_connection_socket = new sf::TcpSocket();
    }
    
    for(auto client = clients.begin(); client != clients.end(); )
    {
        sf::Packet packet;
        sf::Socket::Status status;
        while((status = client->socket->receive(packet)) == sf::Socket::Done)
        {
            uint8_t packet_id;
            packet << packet_id;
            switch(client->state)
            {
            case ClientInfo::State::WaitingForAuthentication:
                switch(packet_id)
                {
                case PacketIDs::request_authentication:
                    for(auto it : node_by_id)
                    {
                        sf::Packet packet;
                        buildCreatePacket(packet, *it.second);
                        client->send(packet);
                    }
                    for(auto it : node_by_id)
                    {
                        if (it.second->multiplayer.replication_links.size() > 0)
                        {
                            sf::Packet packet;
                            packet << PacketIDs::update_object << it.second->multiplayer.getId();
                            for(unsigned int n=0; n<it.second->multiplayer.replication_links.size(); n++)
                            {
                                ReplicationLinkBase* replication_link = it.second->multiplayer.replication_links[n];
                                packet << uint16_t(n);
                                replication_link->handleSend(packet);
                            }
                            client->send(packet);
                        }
                    }
                    {
                        sf::Packet packet;
                        packet << PacketIDs::set_client_id << client->client_id;
                        client->send(packet);
                    }
                    {
                        sf::Packet packet;
                        packet << PacketIDs::change_game_speed << Engine::getInstance()->getGameSpeed();
                        client->send(packet);
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
        if (status == sf::Socket::Disconnected || status == sf::Socket::Error)
        {
            client = clients.erase(client);
        }
        else
        {
            client++;
        }
    }
    
    for(auto client : clients)
    {
        while(client.send_queue.begin() != client.send_queue.end())
        {
            if (client.socket->send(client.send_queue.front()) == sf::Socket::Done)
                client.send_queue.pop_front();
            else
                break;
        }
    }
}

void Server::buildCreatePacket(sf::Packet& packet, SceneNode* node)
{
    auto e = multiplayer::ClassEntry::type_to_name_mapping.find(typeid(*node));
    sp2assert(e != multiplayer::ClassEntry::type_to_name_mapping.end(), (string("No multiplayer class registry for ") + typeid(*node).name()).c_str());
    
    P<SceneNode> parent = node->getParent();

    if (parent)
    {
        packet << PacketIDs::create_object << node->multiplayer.getId() << e->second << parent->multiplayer.getId();
    }
    else
    {
        P<Scene> scene = node->getScene();
        packet << PacketIDs::create_scene << node->multiplayer.getId() << scene->getSceneName();
    }
}

void Server::addNewObject(SceneNode* node)
{
    node->multiplayer.id = next_object_id;
    next_object_id++;
    new_nodes.add(node);
}

void Server::sendToAllConnectedClients(sf::Packet& packet)
{
    for(auto client : clients)
    {
        if (client.state != ClientInfo::State::Connected)
            continue;
        client.send(packet);
    }
}

};//!namespace multiplayer
};//!namespace sp
