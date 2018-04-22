#ifndef SP2_MULTIPLAYER_REPLICATION_H
#define SP2_MULTIPLAYER_REPLICATION_H

#include <SFML/Network/Packet.hpp>

namespace sp {
namespace multiplayer {

class ReplicationLinkBase : NonCopyable
{
public:
    virtual ~ReplicationLinkBase() {}
    virtual bool isChanged() = 0;
    virtual void initialSend(sf::Packet& packet) { send(packet); }
    virtual void send(sf::Packet& packet) = 0;
    virtual void receive(sf::Packet& packet) = 0;
};

template<typename T> class ReplicationLink : public ReplicationLinkBase
{
public:
    ReplicationLink(T& value)
    : value(value), previous_value(value)
    {
    }

    virtual bool isChanged() override
    {
        if (value == previous_value)
            return false;
        previous_value = value;
        return true;
    }
    
    virtual void send(sf::Packet& packet) override
    {
        packet << value;
    }
    
    virtual void receive(sf::Packet& packet) override
    {
        packet >> value;
    }
private:
    T& value;
    T previous_value;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_REPLICATION_H
