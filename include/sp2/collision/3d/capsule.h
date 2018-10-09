#ifndef SP2_COLLISION_3D_CAPSULE_H
#define SP2_COLLISION_3D_CAPSULE_H

#include <sp2/collision/3d/shape.h>

namespace sp {
namespace collision {

class Capsule3D : public Shape3D
{
public:
    enum class Direction
    {
        X, Y, Z
    };

    Capsule3D(Direction direction, double radius, double height);
private:
    Direction direction;
    double radius;
    double height;

    virtual btCollisionShape* createShape() const override;
};

};//namespace collision
};//namespace sp

#endif//SP2_COLLISION_3D_CAPSULE_H
