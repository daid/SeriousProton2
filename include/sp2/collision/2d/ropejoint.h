#ifndef SP2_COLLISION_2D_ROPEJOINT_H
#define SP2_COLLISION_2D_ROPEJOINT_H

#include <sp2/collision/2d/joint.h>

namespace sp {
namespace collision {

class RopeJoint2D : public Joint2D
{
public:
    RopeJoint2D(sp::P<sp::Node> node_a, sp::Vector2d local_point_a, sp::P<sp::Node> node_b, sp::Vector2d local_point_b, float max_length);
    
    void setMaxLength(float max_length);
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_JOINT_H
