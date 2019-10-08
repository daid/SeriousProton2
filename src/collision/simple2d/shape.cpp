#include <sp2/collision/simple2d/shape.h>
#include <sp2/collision/simple2d/simple2dBackend.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>


namespace sp {
namespace collision {

void Simple2DShape::create(Node* node) const
{
    if (!getCollisionBackend(node))
        setCollisionBackend(node, new collision::Simple2DBackend());
    sp2assert(dynamic_cast<collision::Simple2DBackend*>(getCollisionBackend(node)), "Not having a Box2D collision backend, while already having a collision backend. Trying to mix different types of collision?");
    Simple2DBackend* backend = static_cast<collision::Simple2DBackend*>(getCollisionBackend(node));

    sp2assert(node->getParent() == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");
    
    if (getCollisionBody(node))
        backend->destroyBody(getCollisionBody(node));
    setCollisionBody(node, backend->createBody(node, *this));
}

}//namespace collision
}//namespace sp