#include <sp2/collision/3d/box.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

Box3D::Box3D(Vector3d size)
: size(size)
{
}

btCollisionShape* Box3D::createShape() const
{
    return new btBoxShape(btVector3(size.x / 2.0, size.y / 2.0, size.z / 2.0));
}

};//namespace collision
};//namespace sp
