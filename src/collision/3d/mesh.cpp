#include <sp2/collision/3d/mesh.h>
#include <sp2/logging.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

Mesh3D::Mesh3D(std::vector<Vector3f>&& vertices, std::vector<int>&& indices)
: vertices(vertices), indices(indices)
{
    triangle_index_array = new btTriangleIndexVertexArray(this->indices.size() / 3, const_cast<int*>(&this->indices[0]), sizeof(int) * 3, this->vertices.size(), &this->vertices[0].x, sizeof(sp::Vector3f));
}

btCollisionShape* Mesh3D::createShape() const
{
    return new btBvhTriangleMeshShape(triangle_index_array, false);
}

};//namespace collision
};//namespace sp
