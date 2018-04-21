#include <sp2/multiplayer/client.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
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
        case PacketIDs::request_authentication:
            {
                sf::Packet reply;
                reply << PacketIDs::request_authentication << PacketIDs::magic_sp2_value;
                send(reply);
            }
            break;
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
