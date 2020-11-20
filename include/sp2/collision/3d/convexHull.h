#ifndef SP2_COLLISION_3D_CONVEX_HULL_H
#define SP2_COLLISION_3D_CONVEX_HULL_H

#include <vector>

#include <sp2/collision/3d/shape.h>
#include <sp2/math/vector3.h>


namespace sp {
namespace collision {

class ConvexHull3D : public Shape3D
{
public:
#ifdef BT_USE_DOUBLE_PRECISION
    using Vector3 = Vector3d;
#else
    using Vector3 = Vector3f;
#endif

    ConvexHull3D(std::vector<Vector3>&& vertices);

private:
    virtual btCollisionShape* createShape() const override;
    
    std::vector<Vector3> vertices;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_3D_CONVEX_HULL_H
