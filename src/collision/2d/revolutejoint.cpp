#include <sp2/collision/2d/revolutejoint.h>

#include <private/collision/box2dVector.h>
#include <Box2D/Box2D.h>

namespace sp {
namespace collision {

RevoluteJoint2D::RevoluteJoint2D(sp::P<sp::Node> node_a, sp::Vector2d local_point_a, sp::P<sp::Node> node_b, sp::Vector2d local_point_b)
{
    b2RevoluteJointDef joint_def;
    joint_def.localAnchorA = toVector<double>(local_point_a);
    joint_def.localAnchorB = toVector<double>(local_point_b);
    create(&joint_def, node_a, node_b);
}

};//namespace collision
};//namespace sp
