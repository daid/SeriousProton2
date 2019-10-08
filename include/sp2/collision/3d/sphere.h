#ifndef SP2_COLLISION_3D_SPHERE_H
#define SP2_COLLISION_3D_SPHERE_H

#include <sp2/collision/3d/shape.h>

namespace sp {
namespace collision {

class Sphere3D : public Shape3D
{
public:
    Sphere3D(double radius);
private:
    double radius;

    virtual btCollisionShape* createShape() const override;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_3D_SPHERE_H
