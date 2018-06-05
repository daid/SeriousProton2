#include <sp2/collision/3d/shape.h>
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
    if (!scene->collision_world3d)
    {
        scene->collision_configuration3d = new btDefaultCollisionConfiguration();
        scene->collision_dispatcher3d = new btCollisionDispatcher(scene->collision_configuration3d);
        scene->collision_broadphase3d = new btDbvtBroadphase();
        scene->collision_solver3d = new btSequentialImpulseConstraintSolver();
        scene->collision_world3d = new btDiscreteDynamicsWorld(scene->collision_dispatcher3d, scene->collision_broadphase3d, scene->collision_solver3d, scene->collision_configuration3d);
        scene->collision_world3d->setGravity(btVector3(0, 0, 0));
    }

    sp2assert(node->collision_body2d == nullptr, "When setting a 3D collision shape, the node should not have a 2D collision shape");
    sp2assert(node->parent == scene->getRoot(), "3D collision shapes can only be added to top level nodes.");

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
    node->getScene()->collision_world3d->addRigidBody(body, filter_category, filter_mask);
    
    node->collision_body3d = body;
}

};//namespace collision
};//namespace sp
