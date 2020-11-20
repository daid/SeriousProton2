#include <sp2/collision/3d/convexHull.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

ConvexHull3D::ConvexHull3D(std::vector<Vector3>&& vertices)
: vertices(std::move(vertices))
{
}

btCollisionShape* ConvexHull3D::createShape() const
{
    return new btConvexHullShape(&vertices[0].x, vertices.size(), sizeof(Vector3));
}

}//namespace collision
}//namespace sp
