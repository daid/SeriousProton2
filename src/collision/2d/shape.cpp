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
    if (!node->getScene()->collision_backend)
        node->getScene()->collision_backend = new collision::Box2DBackend();
    sp2assert(dynamic_cast<collision::Box2DBackend*>(node->getScene()->collision_backend), "Not having a Box2D collision backend, while already having a collision backend. Trying to mix different types of collision?");
    b2World* world = static_cast<collision::Box2DBackend*>(node->getScene()->collision_backend)->world;

    sp2assert(node->parent == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");

    if (!node->collision_body)
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
        node->collision_body = world->CreateBody(&body_def);
    }
    
    b2Body* body = static_cast<b2Body*>(node->collision_body);
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
