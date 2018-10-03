#ifndef SP2_MULTIPLAYER_REPLICATION_H
#define SP2_MULTIPLAYER_REPLICATION_H

#include <sp2/multiplayer/nodeRegistry.h>
#include <SFML/Network/Packet.hpp>

template<typename=std::enable_if<!std::is_same<uint64_t, sf::Uint64>::value>> static inline sf::Packet& operator<<(sf::Packet& packet, const uint64_t& n) { return packet << sf::Uint64(n); }
template<typename=std::enable_if<!std::is_same<uint64_t, sf::Uint64>::value>> static inline sf::Packet& operator>>(sf::Packet& packet, uint64_t& n) { sf::Uint64 _n; return packet >> _n; n = _n; }

namespace sp {
namespace multiplayer {

class ReplicationLinkBase : NonCopyable
{
public:
    virtual ~ReplicationLinkBase() {}
    virtual bool isChanged() = 0;
    virtual void initialSend(NodeRegistry& registry, sf::Packet& packet) { send(registry, packet); }
    virtual void send(NodeRegistry& registry, sf::Packet& packet) = 0;
    virtual void receive(NodeRegistry& registry, sf::Packet& packet) = 0;
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

template<class T> class ReplicationLink<P<T>> : public ReplicationLinkBase
{
public:
    ReplicationLink(P<T>& object)
    : object(object), previous_id(object ? object.multiplayer.getId() : 0)
    {
    }

    virtual bool isChanged() override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        if (id == previous_id)
            return false;
        previous_id = id;
        return true;
    }
    
    virtual void send(NodeRegistry& registry, sf::Packet& packet) override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        packet << sf::Uint64(id);
    }
    
    virtual void receive(NodeRegistry& registry, sf::Packet& packet) override
    {
        uint64_t id;
        packet >> id;
        object = registry.getNode(id);
    }
private:
    P<T>& object;
    uint64_t previous_id;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_REPLICATION_H
