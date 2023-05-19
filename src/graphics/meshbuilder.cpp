#include <sp2/graphics/meshbuilder.h>

namespace sp {

void MeshBuilder::addQuad(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3)
{
    int i = vertices.size();
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 3);
    
    sp::Vector3f normal = ((p1 - p0).cross(p2 - p0)).normalized();
    vertices.emplace_back(p0, normal, sp::Vector2f(0, 1));
    vertices.emplace_back(p1, normal, sp::Vector2f(1, 1));
    vertices.emplace_back(p2, normal, sp::Vector2f(0, 0));
    vertices.emplace_back(p3, normal, sp::Vector2f(1, 0));
}

void MeshBuilder::addQuad(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3, Vector3f normal)
{
    int i = vertices.size();
    indices.reserve(indices.size() + 6);
    indices.emplace_back(i + 0);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 2);
    indices.emplace_back(i + 1);
    indices.emplace_back(i + 3);
    
    vertices.emplace_back(p0, normal, sp::Vector2f(0, 1));
    vertices.emplace_back(p1, normal, sp::Vector2f(1, 1));
    vertices.emplace_back(p2, normal, sp::Vector2f(0, 0));
    vertices.emplace_back(p3, normal, sp::Vector2f(1, 0));
}

std::shared_ptr<MeshData> MeshBuilder::create()
{
    return sp::MeshData::create(std::move(vertices), std::move(indices));
}

}//namespace sp
