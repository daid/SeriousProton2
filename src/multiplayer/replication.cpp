#include <sp2/multiplayer/replication.h>
#include <sp2/scene/node.h>


namespace sp {
namespace multiplayer {

ReplicationDeadReckoning::ReplicationDeadReckoning(Node& node, const DeadReckoningConfig& config)
: node(node), config(config)
{
}

bool ReplicationDeadReckoning::isChanged(float time_delta)
{
    if (last_update_time > 0.0)
        last_update_time -= time_delta;
    if (last_update_time > config.min_update_delay)
        return false;
    
    sp::Vector3d position = node.getPosition3D();
    sp::Quaterniond rotation = node.getRotation3D();
    
    if (last_update_time < config.max_update_delay)
    {
        last_position += last_velocity * double(time_delta);
        
        sp::Vector3d diff = last_position - position;
        if (diff.dot(diff) < config.max_position_diviation * config.max_position_diviation)
        {
            double angle_diff = std::abs(2 * std::acos((rotation * last_rotation.conjugate()).w)) / pi * 180.0;
            if (angle_diff < config.max_angle_diviation)
                return false;
        }
    }

    last_velocity = node.getLinearVelocity3D();
    last_position = position;
    last_rotation = rotation;
    return true;
}

void ReplicationDeadReckoning::initialSend(Base& registry, io::DataBuffer& packet)
{
    send(registry, packet);
}

void ReplicationDeadReckoning::send(Base& registry, io::DataBuffer& packet)
{
    Vector3d pos = node.getPosition3D();
    Vector3d velocity = node.getLinearVelocity3D();
    Quaterniond rotation = node.getRotation3D();
    Vector3d angular_velocity = node.getAngularVelocity3D();
    packet.write(pos.x, pos.y, pos.z, velocity.x, velocity.y, velocity.z);
    packet.write(rotation.x, rotation.y, rotation.z, rotation.w);
    packet.write(angular_velocity.x, angular_velocity.y, angular_velocity.z);
}

void ReplicationDeadReckoning::receive(Base& registry, io::DataBuffer& packet)
{
    Vector3d pos;
    Vector3d velocity;
    Quaterniond rotation;
    Vector3d angular_velocity;
    packet.read(pos.x, pos.y, pos.z, velocity.x, velocity.y, velocity.z);
    packet.read(rotation.x, rotation.y, rotation.z, rotation.w);
    packet.read(angular_velocity.x, angular_velocity.y, angular_velocity.z);
    
    pos += velocity * double(registry.getNetworkDelay());
    node.setPosition(pos);
    node.setLinearVelocity(velocity);
    node.setRotation(rotation);
    node.setAngularVelocity(angular_velocity);
}

};//namespace multiplayer
};//namespace sp
