#ifndef SP2_COLLISION_3D_CYLINDER_H
#define SP2_COLLISION_3D_CYLINDER_H

#include <sp2/collision/3d/shape.h>

namespace sp {
namespace collision {

class Cylinder3D : public Shape3D
{
public:
    enum class Direction
    {
        X, Y, Z
    };

    Cylinder3D(Direction direction, double radius, double height);
private:
    Direction direction;
    double radius;
    double height;

    virtual btCollisionShape* createShape() const override;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_3D_CYLINDER_H
