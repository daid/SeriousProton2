#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/registry.h>
#include <private/multiplayer/packetIDs.h>
#include <sp2/scene/scene.h>
#include <sp2/engine.h>
#include <sp2/assert.h>

#include <SFML/Network/TcpSocket.hpp>

namespace sp {
namespace multiplayer {

Server::Server(int port_nr)
{
    if (new_connection_listener.listen(port_nr) != sf::Socket::Done)
        LOG(Error, "Failed to listen on port: ", port_nr);
    new_connection_listener.setBlocking(false);
    new_connection_socket = new sf::TcpSocket();
    
    next_client_id = 1;
    next_object_id = 1;
}

Server::~Server()
{
    delete new_connection_socket;
}

void Server::recursiveAddNewNodes(Node* node)
{
    if (node->multiplayer.enabled)
    {
        if (node->multiplayer.id == 0)
            addNewObject(node);
        for(Node* child : node->getChildren())
            recursiveAddNewNodes(child);
    }
}

void Server::onUpdate(float delta)
{
    for(Scene* scene : Scene::scenes)
    {
        recursiveAddNewNodes(*scene->getRoot());
    }

    //When creating new objects, we first send out packets for all objects to be created.
    //And then we send out variable value updates. This because else we could update a pointer variable to an object that does not exist yet.
    for(Node* node : new_nodes)
    {
        sf::Packet packet;
        buildCreatePacket(packet, node);
        
        sendToAllConnectedClients(packet);
        
        addNode(node);
    }
    for(Node* node : new_nodes)
    {
        if (node->multiplayer.replication_links.size() > 0)
        {
            sf::Packet packet;
            packet << PacketIDs::update_object << node->multiplayer.getId();
            for(unsigned int n=0; n<node->multiplayer.replication_links.size(); n++)
            {
                ReplicationLinkBase* replication_link = node->multiplayer.replication_links[n];
                packet << uint16_t(n);
                replication_link->initialSend(*this, packet);
            }
            sendToAllConnectedClients(packet);
        }
    }
    new_nodes.clear();
    
    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
    {
        sf::Packet packet;
        packet << PacketIDs::update_object << it->second->multiplayer.getId();
        unsigned int zero_data_size = packet.getDataSize();
        for(unsigned int n=0; n<it->second->multiplayer.replication_links.size(); n++)
        {
            ReplicationLinkBase* replication_link = it->second->multiplayer.replication_links[n];
            if (replication_link->isChanged())
            {
                packet << uint16_t(n);
                replication_link->send(*this, packet);
            }
        }
        if (packet.getDataSize() != zero_data_size)
            sendToAllConnectedClients(packet);
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
        if (client.socket->send(packet) != sf::Socket::Done)
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
            packet >> packet_id;
            switch(client->state)
            {
            case ClientInfo::State::WaitingForAuthentication:
                switch(packet_id)
                {
                case PacketIDs::request_authentication:
                    {
                        sf::Packet send_packet;
                        send_packet << PacketIDs::set_client_id << client->client_id;
                        client->send(send_packet);
                    }
                    {
                        sf::Packet send_packet;
                        send_packet << PacketIDs::change_game_speed << Engine::getInstance()->getGameSpeed();
                        client->send(send_packet);
                    }

                    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
                    {
                        sf::Packet send_packet;
                        buildCreatePacket(send_packet, *it->second);
                        client->send(send_packet);
                    }
                    for(auto it = nodeBegin(); it != nodeEnd(); ++it)
                    {
                        if (it->second->multiplayer.replication_links.size() > 0)
                        {
                            sf::Packet send_packet;
                            send_packet << PacketIDs::update_object << it->first;
                            for(unsigned int n=0; n<it->second->multiplayer.replication_links.size(); n++)
                            {
                                ReplicationLinkBase* replication_link = it->second->multiplayer.replication_links[n];
                                send_packet << uint16_t(n);
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

void Server::buildCreatePacket(sf::Packet& packet, Node* node)
{
    auto e = multiplayer::ClassEntry::type_to_name_mapping.find(typeid(*node));
    sp2assert(e != multiplayer::ClassEntry::type_to_name_mapping.end(), (string("No multiplayer class registry for ") + typeid(*node).name()).c_str());
    
    P<Node> parent = node->getParent();

    if (parent)
    {
        packet << PacketIDs::create_object << node->multiplayer.getId() << e->second << parent->multiplayer.getId();
    }
    else
    {
        //This node is the root node of a scene, so setup our scene information.
        P<Scene> scene = node->getScene();
        packet << PacketIDs::setup_scene << node->multiplayer.getId() << scene->getSceneName();
    }
}

void Server::onDeleted(uint64_t id)
{
    sf::Packet packet;
    packet << PacketIDs::delete_object << id;
    sendToAllConnectedClients(packet);
}

void Server::addNewObject(Node* node)
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
