#include <sp2/collision/2d/shape.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

void Shape2D::create(SceneNode* node) const
{
    if (!node->getScene()->collision_world2d)
    {
        node->getScene()->collision_world2d = new b2World(b2Vec2_zero);
    }
    b2World* world = node->getScene()->collision_world2d;

    sp2assert(node->parent == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");

    if (!node->collision_body2d)
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
        body_def.userData = node;
        node->collision_body2d = world->CreateBody(&body_def);
    }
    
    while(node->collision_body2d->GetFixtureList())
        node->collision_body2d->DestroyFixture(node->collision_body2d->GetFixtureList());
    createFixture(node->collision_body2d);
}

};//!namespace collision
};//!namespace sp
