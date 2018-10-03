#include <sp2/collision/3d/capsule.h>

#include <btBulletDynamicsCommon.h>

namespace sp {
namespace collision {

Capsule3D::Capsule3D(Direction direction, double radius, double height)
: direction(direction), radius(radius), height(height)
{
}

btCollisionShape* Capsule3D::createShape() const
{
    switch(direction)
    {
    case Direction::X:
        return new btCapsuleShapeX(radius, height);
    case Direction::Y:
        return new btCapsuleShape(radius, height);
    case Direction::Z:
    default:
        return new btCapsuleShapeZ(radius, height);
    }
}

};//namespace collision
};//namespace sp
