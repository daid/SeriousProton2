#include <sp2/collision/2d/shape.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>

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
    
    if (!node->collision_body2d)
    {
        b2BodyDef body_def;
        body_def.position = toVector(node->getGlobalPosition2D());
        body_def.angle = node->getGlobalRotation2D();
        switch(type)
        {
        case Type::Sensor:
            body_def.type = b2_staticBody;
            break;
        case Type::Static:
            body_def.type = b2_staticBody;
            break;
        case Type::Dynamic:
            body_def.type = b2_dynamicBody;
            break;
        }
        node->collision_body2d = world->CreateBody(&body_def);
    }
    
    createFixture(node->collision_body2d);
}

};//!namespace collision
};//!namespace sp
