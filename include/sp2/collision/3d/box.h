#ifndef SP2_COLLISION_3D_BOX_H
#define SP2_COLLISION_3D_BOX_H

#include <sp2/math/vector3.h>
#include <sp2/collision/3d/shape.h>

namespace sp {
namespace collision {

class Box3D : public Shape3D
{
public:
    Box3D(Vector3d size);
private:
    Vector3d size;

    virtual btCollisionShape* createShape() const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_3D_BOX_H
