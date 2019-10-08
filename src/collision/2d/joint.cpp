#include <sp2/collision/2d/joint.h>
#include <sp2/collision/2d/box2dBackend.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

Joint2D::Joint2D()
{
    joint = nullptr;
}

void Joint2D::create(b2JointDef* joint_def, sp::P<sp::Node> node_a, sp::P<sp::Node> node_b)
{
    sp2assert(node_a && node_b, "Must supply nodes to a joint.");
    sp2assert(node_a->getScene() == node_b->getScene(), "Scenes of nodes given to a joint must be the same.");
    sp2assert(node_a->collision_body && node_b->collision_body, "Nodes given to a joint need a body.");
    scene = node_a->getScene();
    sp2assert(scene->collision_backend, "No collision backend when creating a joint.");
    
    collision::Box2DBackend* backend = dynamic_cast<collision::Box2DBackend*>(scene->collision_backend);
    sp2assert(backend, "No 2d collision backend when creating a 2d joint.");
    if (joint)
    {
        joint->SetUserData(nullptr);
        backend->world->DestroyJoint(joint);
    }
    
    joint_def->bodyA = static_cast<b2Body*>(node_a->collision_body);
    joint_def->bodyB = static_cast<b2Body*>(node_b->collision_body);
    joint_def->userData = this;

    joint = backend->world->CreateJoint(joint_def);
}

Joint2D::~Joint2D()
{
    if (joint && scene)
    {
        joint->SetUserData(nullptr);
        collision::Box2DBackend* backend = static_cast<collision::Box2DBackend*>(scene->collision_backend);
        backend->world->DestroyJoint(joint);
    }
}

}//namespace collision
}//namespace sp
