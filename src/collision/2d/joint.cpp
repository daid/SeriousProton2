#include <sp2/collision/2d/joint.h>
#include <sp2/scene/scene.h>
#include <sp2/assert.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

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
    sp2assert(node_a->collision_body2d && node_b->collision_body2d, "Nodes given to a joint need a body.");
    
    scene = node_a->getScene();
    joint_def->bodyA = node_a->collision_body2d;
    joint_def->bodyB = node_b->collision_body2d;
    joint_def->userData = this;

    joint = scene->collision_world2d->CreateJoint(joint_def);
}

Joint2D::~Joint2D()
{
    if (joint && scene)
    {
        joint->SetUserData(nullptr);
        scene->collision_world2d->DestroyJoint(joint);
    }
}

};//namespace collision
};//namespace sp
