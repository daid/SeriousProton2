#ifndef SP2_COLLISION_2D_JOINT_H
#define SP2_COLLISION_2D_JOINT_H

#include <sp2/pointer.h>
#include <sp2/scene/node.h>

class b2Joint;
struct b2JointDef;

namespace sp {
namespace collision {

class Joint2D : public AutoPointerObject
{
public:
    virtual ~Joint2D();
protected:
    Joint2D();

    void create(b2JointDef* joint_def, sp::P<sp::Node> node_a, sp::P<sp::Node> node_b);

    sp::P<sp::Scene> scene;
    b2Joint* joint;
    
    friend class DestructionListener;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_2D_JOINT_H
