#ifndef SP2_MULTIPLAYER_REPLICATION_H
#define SP2_MULTIPLAYER_REPLICATION_H

#include <sp2/multiplayer/nodeRegistry.h>
#include <sp2/io/dataBuffer.h>
#include <sp2/math/vector.h>
#include <sp2/math/quaternion.h>


namespace sp {
namespace multiplayer {

class ReplicationLinkBase : NonCopyable
{
public:
    virtual ~ReplicationLinkBase() {}
    virtual bool isChanged(float time_delta) = 0;
    virtual void initialSend(NodeRegistry& registry, io::DataBuffer& packet) { send(registry, packet); }
    virtual void send(NodeRegistry& registry, io::DataBuffer& packet) = 0;
    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet) = 0;
};

template<typename T> class ReplicationLink : public ReplicationLinkBase
{
public:
    ReplicationLink(T& value)
    : value(value), previous_value(value), timeout(0.0), max_update_interval(0.0)
    {
    }

    ReplicationLink(T& value, float max_update_interval)
    : value(value), previous_value(value), timeout(0.0), max_update_interval(max_update_interval)
    {
    }

    virtual bool isChanged(float time_delta) override
    {
        if (timeout > 0.0)
            timeout -= time_delta;
        if (timeout > 0.0)
            return false;
        if (value == previous_value)
            return false;
        previous_value = value;
        timeout = max_update_interval;
        return true;
    }

    virtual void send(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        packet.write(value);
    }

    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        packet.read(value);
    }
private:
    T& value;
    T previous_value;
    float timeout;
    float max_update_interval;
};

template<class T> class ReplicationLink<P<T>> : public ReplicationLinkBase
{
public:
    ReplicationLink(P<T>& object)
    : object(object), previous_id(object ? object.multiplayer.getId() : 0)
    {
    }

    virtual bool isChanged(float time_delta) override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        if (id == previous_id)
            return false;
        previous_id = id;
        return true;
    }
    
    virtual void send(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        uint64_t id = object ? object.multiplayer.getId() : 0;
        packet.write(id);
    }
    
    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet) override
    {
        uint64_t id;
        packet.read(id);
        object = registry.getNode(id);
    }
private:
    P<T>& object;
    uint64_t previous_id;
};

struct DeadReckoningConfig
{
    float min_update_delay = 0.0;
    float max_update_delay = 5.0;
    double max_position_diviation = 0.1;
    double max_angle_diviation = 3.0;
};
class ReplicationDeadReckoning : public ReplicationLinkBase
{
public:
    ReplicationDeadReckoning(Node& node, const DeadReckoningConfig& config);

    virtual bool isChanged(float time_delta);
    virtual void initialSend(NodeRegistry& registry, io::DataBuffer& packet);
    virtual void send(NodeRegistry& registry, io::DataBuffer& packet);
    virtual void receive(NodeRegistry& registry, io::DataBuffer& packet);

private:
    Node& node;
    DeadReckoningConfig config;
    float last_update_time = 0.0;

    Vector3d last_position;
    Vector3d last_velocity;
    Quaterniond last_rotation;
};

};//namespace multiplayer
};//namespace sp

#endif//SP2_MULTIPLAYER_REPLICATION_H
