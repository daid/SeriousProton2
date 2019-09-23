#include <sp2/collision/3d/sphere.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

Sphere3D::Sphere3D(double radius)
: radius(radius)
{
}

btCollisionShape* Sphere3D::createShape() const
{
    return new btSphereShape(radius);
}

};//namespace collision
};//namespace sp
