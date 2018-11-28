#include <sp2/collision/3d/shape.h>
#include <sp2/collision/3d/bullet3dBackend.h>
#include <sp2/scene/node.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/bulletVector.h>

#include <btBulletDynamicsCommon.h>

namespace sp {
namespace collision {

void Shape3D::create(Node* node) const
{
    P<Scene> scene = node->getScene();

    sp2assert(node->parent == scene->getRoot(), "3D collision shapes can only be added to top level nodes.");

    if (!node->getScene()->collision_backend)
        node->getScene()->collision_backend = new collision::BulletBackend();
    sp2assert(dynamic_cast<collision::BulletBackend*>(node->getScene()->collision_backend), "Not having a Bullet collision backend, while already having a collision backend. Trying to mix different types of collision?");
    btDiscreteDynamicsWorld* world = static_cast<collision::BulletBackend*>(node->getScene()->collision_backend)->world;

    btCollisionShape* shape = createShape();
    
    btScalar mass = 0.0;
    if (type == Type::Dynamic || type == Type::Kinematic)
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
    
    node->collision_body = body;
}

};//namespace collision
};//namespace sp
