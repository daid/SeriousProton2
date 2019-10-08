#ifndef SP2_COLLISION_3D_MESH_H
#define SP2_COLLISION_3D_MESH_H

#include <vector>

#include <sp2/collision/3d/shape.h>
#include <sp2/math/vector3.h>

class btTriangleIndexVertexArray;

namespace sp {
namespace collision {

/**
    NOTE: Unlike the other collision objects, the Mesh3D needs to remain in memory as long as any object is using this as collision shape.
            else you will get a crash from the collision engine, as it will try to access the stored vertices/indices.
 */
class Mesh3D : public Shape3D
{
public:
    Mesh3D(std::vector<Vector3f>&& vertices, std::vector<int>&& indices);

private:
    virtual btCollisionShape* createShape() const override;
    
    btTriangleIndexVertexArray* triangle_index_array = nullptr;

    std::vector<Vector3f> vertices;
    std::vector<int> indices;
};

}//namespace collision
}//namespace sp

#endif//SP2_COLLISION_3D_MESH_H
