#include <sp2/collision/3d/shape.h>
#include <sp2/collision/3d/bullet3dBackend.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/bulletVector.h>
#include <private/collision/bullet.h>


namespace sp {
namespace collision {

void Shape3D::create(Node* node) const
{
    P<Scene> scene = node->getScene();

    sp2assert(node->getParent() == scene->getRoot(), "3D collision shapes can only be added to top level nodes.");

    if (!getCollisionBackend(node))
        setCollisionBackend(node, new collision::BulletBackend());
    sp2assert(dynamic_cast<collision::BulletBackend*>(getCollisionBackend(node)), "Not having a Bullet collision backend, while already having a collision backend. Trying to mix different types of collision?");
    btDiscreteDynamicsWorld* world = static_cast<collision::BulletBackend*>(getCollisionBackend(node))->world;

    btCollisionShape* shape = createShape();
    
    btScalar mass = 0.0;
    if (type == Type::Dynamic)
        mass = density;
    
    btVector3 local_inertia(0, 0, 0);
    if (mass > 0)
        shape->calculateLocalInertia(mass, local_inertia);
    
    btRigidBody::btRigidBodyConstructionInfo construction_info(mass, nullptr, shape, local_inertia);
    construction_info.m_startWorldTransform.setOrigin(toVector(node->getPosition3D()));
    construction_info.m_startWorldTransform.setRotation(toQuadernion(node->getRotation3D()));

    construction_info.m_linearDamping = linear_damping;
    construction_info.m_angularDamping = angular_damping;
    
    construction_info.m_friction = friction;
    construction_info.m_restitution = restitution;
    
    //Default threshold for sleeping is quite high, we lower this to prevent slow moving objects from suddenly stopping.
    construction_info.m_linearSleepingThreshold = 0.01;
    construction_info.m_angularSleepingThreshold = 0.5 / 180.0 * pi;

    btRigidBody* body = new btRigidBody(construction_info);
    body->setUserPointer(node);
    switch(type)
    {
    case Type::Sensor:
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        break;
    case Type::Static:
        break;
    case Type::Kinematic:
        body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        break;
    case Type::Dynamic:
        break;
    }
    if (fixed_rotation)
        body->setAngularFactor(0.0);
    world->addRigidBody(body, filter_category, filter_mask);
    
    setCollisionBody(node, body);
}

}//namespace collision
}//namespace sp
