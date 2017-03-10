#ifndef SP2_MULTIPLAYER_REPLICATION_H
#define SP2_MULTIPLAYER_REPLICATION_H

#include <SFML/Network/Packet.hpp>

namespace sp {
namespace multiplayer {

class ReplicationLinkBase
{
public:
    virtual ~ReplicationLinkBase() {}
    virtual bool isChanged() = 0;
    virtual void handleSend(sf::Packet& packet) = 0;
    virtual void handleReceived(sf::Packet& packet) = 0;
};

template<typename T> class ReplicationLink
{
public:
    virtual bool isChanged() override
    {
        if (*value_ptr == previous_value)
            return false;
        previous_value = *value_ptr;
        return true;
    }

    virtual void handleSend(sf::Packet& packet) override
    {
        packet << *value_ptr;
    }
    
    virtual void handleReceived(sf::Packet& packet) override
    {
        packet >> *value_ptr;
    }
private:
    T* value_ptr;
    T previous_value;
};

};//!namespace multiplayer
};//!namespace sp

#endif//SP2_MULTIPLAYER_REPLICATION_H
