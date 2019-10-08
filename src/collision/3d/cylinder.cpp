#include <sp2/collision/3d/cylinder.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

Cylinder3D::Cylinder3D(Direction direction, double radius, double height)
: direction(direction), radius(radius), height(height)
{
}

btCollisionShape* Cylinder3D::createShape() const
{
    switch(direction)
    {
    case Direction::X:
        return new btCylinderShapeX(btVector3(height / 2.0, radius, radius));
    case Direction::Y:
        return new btCylinderShape(btVector3(radius, height / 2.0, radius));
    case Direction::Z:
    default:
        return new btCylinderShapeZ(btVector3(radius, radius, height / 2.0));
    }
}

}//namespace collision
}//namespace sp
