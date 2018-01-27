#include <sp2/graphics/opengl.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/logging.h>
#include <limits>
#include <string.h>

const unsigned int NO_BUFFER = std::numeric_limits<unsigned int>::max();

namespace sp {

MeshData::MeshData(Type type)
: type(type)
{
    vbo = NO_BUFFER;
    dirty = true;
    revision = 0;
}

MeshData::MeshData(std::vector<Vertex>&& vertices, Type type)
: MeshData(type)
{
    this->vertices = std::move(vertices);
}

MeshData::~MeshData()
{
    if (vbo != NO_BUFFER)
        glDeleteBuffers(1, &vbo);
}

void MeshData::render()
{
    if (vertices.size() < 1)
        return;
    if (vbo == NO_BUFFER)
    {
        glGenBuffers(1, &vbo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (dirty)
    {
        switch(type)
        {
        case Type::Static: glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW); break;
        case Type::Dynamic: glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW); break;
        }
        dirty = false;
    }
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void MeshData::update(std::vector<Vertex>&& vertices)
{
    this->vertices = std::move(vertices);
    dirty = true;
    revision++;
}

std::shared_ptr<MeshData> MeshData::create(Vertices&& vertices, Type type)
{
    return std::make_shared<MeshData>(std::forward<Vertices>(vertices), type);
}

std::shared_ptr<MeshData> MeshData::createQuad(sp::Vector2f size)
{
    size *= 0.5f;
    
    Vertices vertices;
    vertices.emplace_back(sf::Vector3f(-size.x, -size.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f( size.x,  size.y, 0.0f), sp::Vector2f(1, 0));
    
    return std::make_shared<MeshData>(std::move(vertices));
}

std::shared_ptr<MeshData> MeshData::createDoubleSidedQuad(sp::Vector2f size)
{
    size *= 0.5f;
    
    Vertices vertices;
    vertices.emplace_back(sf::Vector3f(-size.x, -size.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f( size.x,  size.y, 0.0f), sp::Vector2f(1, 0));

    vertices.emplace_back(sf::Vector3f(-size.x, -size.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( size.x,  size.y, 0.0f), sp::Vector2f(1, 0));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    
    return std::make_shared<MeshData>(std::move(vertices));
}

}//!namespace sp
