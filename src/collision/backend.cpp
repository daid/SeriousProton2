#include <sp2/collision/backend.h>
#include <sp2/scene/node.h>


namespace sp {
namespace collision {

void* Backend::getCollisionBody(sp::P<sp::Node>& node)
{
    return node->collision_body;
}

void Backend::setCollisionBody(sp::P<sp::Node>& node, void* body)
{
    node->collision_body = body;
}

void Backend::modifyPositionByPhysics(Node* node, sp::Vector2d position, double rotation)
{
    node->modifyPositionByPhysics(position, rotation);
}

void Backend::modifyPositionByPhysics(Node* node, sp::Vector3d position, Quaterniond rotation)
{
    node->modifyPositionByPhysics(position, rotation);
}

};//namespace collision
};//namespace sp
