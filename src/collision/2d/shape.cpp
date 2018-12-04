#include <sp2/collision/2d/shape.h>
#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

void Shape2D::create(Node* node) const
{
    if (!getCollisionBackend(node))
        setCollisionBackend(node, new collision::Box2DBackend());
    sp2assert(dynamic_cast<collision::Box2DBackend*>(getCollisionBackend(node)), "Not having a Box2D collision backend, while already having a collision backend. Trying to mix different types of collision?");
    b2World* world = static_cast<collision::Box2DBackend*>(getCollisionBackend(node))->world;

    sp2assert(node->getParent() == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");

    if (!getCollisionBody(node))
    {
        b2BodyDef body_def;
        body_def.position = toVector(node->getGlobalPosition2D());
        body_def.angle = node->getGlobalRotation2D() / 180.0 * pi;
        body_def.linearDamping = linear_damping;
        body_def.angularDamping = angular_damping;
        switch(type)
        {
        case Type::Sensor:
            body_def.type = b2_dynamicBody;
            break;
        case Type::Static:
            body_def.type = b2_staticBody;
            break;
        case Type::Kinematic:
            body_def.type = b2_kinematicBody;
            break;
        case Type::Dynamic:
            body_def.type = b2_dynamicBody;
            break;
        }
        body_def.fixedRotation = fixed_rotation;
        body_def.userData = node;
        setCollisionBody(node, world->CreateBody(&body_def));
    }
    
    b2Body* body = static_cast<b2Body*>(getCollisionBody(node));
    while(body->GetFixtureList())
        body->DestroyFixture(body->GetFixtureList());
    createFixture(body);
}

void Shape2D::createFixtureOnBody(b2Body* body, b2Shape* shape) const
{
    b2FixtureDef shapeDef;
    shapeDef.shape = shape;
    shapeDef.density = density;
    shapeDef.friction = friction;
    shapeDef.restitution = restitution;
    shapeDef.isSensor = (type == Type::Sensor);
    shapeDef.userData = body->GetUserData();
    shapeDef.filter.categoryBits = filter_category;
    shapeDef.filter.maskBits = filter_mask;
    body->CreateFixture(&shapeDef);
}

};//namespace collision
};//namespace sp
