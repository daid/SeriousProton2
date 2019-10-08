#ifndef SP2_COLLISION_2D_REVOLUTEJOINT_H
#define SP2_COLLISION_2D_REVOLUTEJOINT_H

#include <sp2/collision/2d/joint.h>

namespace sp {
namespace collision {

class RevoluteJoint2D : public Joint2D
{
public:
    RevoluteJoint2D(sp::P<sp::Node> node_a, sp::Vector2d local_point_a, sp::P<sp::Node> node_b, sp::Vector2d local_point_b);
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_REVOLUTEJOINT_H
