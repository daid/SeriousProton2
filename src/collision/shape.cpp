#include <sp2/collision/shape.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>


namespace sp {
namespace collision {

Backend* Shape::getCollisionBackend(Node* node) const
{
    return node->scene->collision_backend;
}

void Shape::setCollisionBackend(Node* node, Backend* backend) const
{
    node->scene->collision_backend = backend;
}

void* Shape::getCollisionBody(Node* node) const
{
    return node->collision_body;
}

void Shape::setCollisionBody(Node* node, void* body) const
{
    node->collision_body = body;
}

}//namespace collision
}//namespace sp
