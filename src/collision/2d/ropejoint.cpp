#include <sp2/collision/2d/ropejoint.h>

#include <private/collision/box2dVector.h>
#include <private/collision/box2d.h>


namespace sp {
namespace collision {

RopeJoint2D::RopeJoint2D(sp::P<sp::Node> node_a, sp::Vector2d local_point_a, sp::P<sp::Node> node_b, sp::Vector2d local_point_b, float max_length)
{
    b2RopeJointDef joint_def;
    joint_def.localAnchorA = toVector<double>(local_point_a);
    joint_def.localAnchorB = toVector<double>(local_point_b);
    joint_def.maxLength = max_length;
    joint_def.collideConnected = true;
    create(&joint_def, node_a, node_b);
}

void RopeJoint2D::setMaxLength(float max_length)
{
    if (joint)
        ((b2RopeJoint*)joint)->SetMaxLength(max_length);
}

}//namespace collision
}//namespace sp
