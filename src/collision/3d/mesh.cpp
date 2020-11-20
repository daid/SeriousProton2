#include <sp2/collision/3d/mesh.h>
#include <sp2/logging.h>
#include <sp2/assert.h>

#include <private/collision/bullet.h>


namespace sp {
namespace collision {

Mesh3D::Mesh3D(std::vector<Vector3>&& vertices, std::vector<int>&& indices)
: vertices(vertices), indices(indices)
{
    triangle_index_array = new btTriangleIndexVertexArray(this->indices.size() / 3, const_cast<int*>(&this->indices[0]), sizeof(int) * 3, this->vertices.size(), &this->vertices[0].x, sizeof(Vector3));
}

btCollisionShape* Mesh3D::createShape() const
{
    sp2assert(type != Type::Dynamic, "Mesh3D cannot be dynamic, bullet physics limitation.");
    return new btBvhTriangleMeshShape(triangle_index_array, false);
}

}//namespace collision
}//namespace sp
