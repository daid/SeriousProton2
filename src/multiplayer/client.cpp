#include <sp2/multiplayer/client.h>
#include <sp2/multiplayer/registry.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/engine.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <private/multiplayer/packetIDs.h>

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>

namespace sp {
namespace multiplayer {

Client::Client(string hostname, int port_nr)
{
    socket.setBlocking(false);
    socket.connect(sf::IpAddress(hostname), port_nr);
    
    state = State::Connecting;
}

Client::~Client()
{
}
    
void Client::onUpdate(float delta)
{
    sf::Packet packet;
    
    sf::Socket::Status status;
    while((status = socket.receive(packet)) == sf::Socket::Done)
    {
        uint8_t command_id;
        packet >> command_id;
        switch(command_id)
        {
        case PacketIDs::request_authentication:{
            sf::Packet reply;
            reply << PacketIDs::request_authentication << PacketIDs::magic_sp2_value;
            send(reply);
            }break;
        case PacketIDs::set_client_id:{
            packet >> client_id;
            }break;
            
        case PacketIDs::change_game_speed:{
            float new_gamespeed;
            packet >> new_gamespeed;
            sp::Engine::getInstance()->setGameSpeed(new_gamespeed);
            }break;
    
        case PacketIDs::create_object:{
            uint64_t id;
            string class_name;
            uint64_t parent;
            packet >> id >> class_name >> parent;
            auto it = multiplayer::ClassEntry::name_to_create_mapping.find(class_name);
            if (it == multiplayer::ClassEntry::name_to_create_mapping.end())
            {
                LOG(Error, "Got class", class_name, "but no way to create it.");
            }
            else
            {
                P<Node> parent_node = getNode(parent);
                if (!parent_node)
                {
                    LOG(Error, "Cannot find parent to create multiplayer object for", class_name, parent);
                }
                else
                {
                    P<Node> new_node = it->second(parent_node);
                    new_node->multiplayer.id = id;
                    addNode(new_node);
                }
            }
            }break;
        case PacketIDs::update_object:{
            uint64_t id;
            packet >> id;
            P<Node> node = getNode(id);
            if (node)
            {
                uint16_t idx;
                while(packet >> idx)
                {
                    if (idx < node->multiplayer.replication_links.size())
                        node->multiplayer.replication_links[idx]->receive(*this, packet);
                }
            }
            }break;
        case PacketIDs::delete_object:{
            uint64_t id;
            packet >> id;
            P<Node> node = getNode(id);
            if (node)
                delete *node;
            }break;

        case PacketIDs::setup_scene:{
            uint64_t id;
            string scene_name;
            packet >> id >> scene_name;
            sp::P<Scene> scene = Scene::get(scene_name);
            if (scene)
            {
                scene->getRoot()->multiplayer.id = id;
                addNode(scene->getRoot());
            }
            else
            {
                LOG(Error, "Server send data for scene", scene_name, "but could not find the scene, this most likely will result in missing parents as well.");
            }
            }break;

        case PacketIDs::alive:
            break;
        default:
            LOG(Warning, "Received unknown packet:", command_id);
        }
    }
    if (status == sf::Socket::Disconnected || status == sf::Socket::Error)
        state = State::Disconnected;

    while(send_queue.begin() != send_queue.end())
    {
        if (socket.send(send_queue.front()) == sf::Socket::Done)
            send_queue.pop_front();
        else
            break;
    }
    
    cleanDeletedNodes();
}

void Client::send(sf::Packet& packet)
{
    if (send_queue.begin() == send_queue.end())
    {
        if (socket.send(packet) == sf::Socket::Partial)
            send_queue.push_back(packet);
    }
    else
    {
        send_queue.push_back(packet);
    }
}


};//!namespace multiplayer
};//!namespace sp
