#include <sp2/collision/2d/shape.h>
#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

void Shape2D::create(Node* node) const
{
    if (!getCollisionBackend(node))
        setCollisionBackend(node, new collision::Box2DBackend());
    sp2assert(dynamic_cast<collision::Box2DBackend*>(getCollisionBackend(node)), "Not having a Box2D collision backend, while already having a collision backend. Trying to mix different types of collision?");
    auto backend = static_cast<collision::Box2DBackend*>(getCollisionBackend(node));

    sp2assert(node->getParent() == node->getScene()->getRoot(), "2D collision shapes can only be added to top level nodes.");

    if (!getCollisionBody(node))
    {
        b2BodyDef body_def = b2DefaultBodyDef();
        body_def.position = toVector(node->getGlobalPosition2D());
        body_def.rotation = b2MakeRot(node->getGlobalRotation2D() / 180.0 * pi);
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
        auto bodyId = b2CreateBody(backend->world, &body_def);
        setCollisionBody(node, backend->addBodyId(bodyId));
    }
    else
    {
        b2BodyId body = backend->getBodyId(getCollisionBody(node));
        
        b2Body_SetLinearDamping(body, linear_damping);
        b2Body_SetAngularDamping(body, angular_damping);
        switch(type)
        {
        case Type::Sensor:
            b2Body_SetType(body, b2_dynamicBody);
            break;
        case Type::Static:
            b2Body_SetType(body, b2_staticBody);
            break;
        case Type::Kinematic:
            b2Body_SetType(body, b2_kinematicBody);
            break;
        case Type::Dynamic:
            b2Body_SetType(body, b2_dynamicBody);
            break;
        }
        b2Body_SetFixedRotation(body, fixed_rotation);
    }
    
    b2BodyId body = backend->getBodyId(getCollisionBody(node));
    while(b2Body_GetShapeCount(body) > 0)
    {
        b2ShapeId shapes[1];
        b2Body_GetShapes(body, shapes, 1);
        b2DestroyShape(shapes[0], false);
    }
    createShape(body);
}

b2ShapeDef Shape2D::getShapeDef(b2BodyId body) const
{
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    shapeDef.material.restitution = restitution;
    shapeDef.isSensor = (type == Type::Sensor);
    shapeDef.userData = b2Body_GetUserData(body);
    shapeDef.filter.categoryBits = filter_category;
    shapeDef.filter.maskBits = filter_mask;
    // shapeDef.enableContactEvents = true;
    // shapeDef.enableSensorEvents = true;
    return shapeDef;
}

}//namespace collision
}//namespace sp
