#include <sp2/graphics/opengl.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/logging.h>
#include <limits>
#include <string.h>

const unsigned int NO_BUFFER = std::numeric_limits<unsigned int>::max();

namespace sp {

MeshData::MeshData()
{
    vertices = nullptr;
    vertex_count = 0;
    vbo = NO_BUFFER;
}

MeshData::MeshData(std::vector<Vertex>& vertices)
{
    this->vertices = new Vertex[vertices.size()];
    memcpy(this->vertices, vertices.data(), sizeof(Vertex) * vertices.size());
    vertex_count = vertices.size();
    vbo = NO_BUFFER;
}

MeshData::~MeshData()
{
    if (vertices) delete vertices;
    if (vbo != NO_BUFFER)
        glDeleteBuffers(1, &vbo);
}

void MeshData::render()
{
    if (!vertex_count)
        return;
    if (vbo == NO_BUFFER)
    {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertex_count, &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glNormalPointer(GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::shared_ptr<MeshData> MeshData::createQuad(sp::Vector2f size)
{
    size *= 0.5f;
    
    std::vector<Vertex> vertices;
    vertices.emplace_back(sf::Vector3f(-size.x, -size.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(-size.x,  size.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( size.x, -size.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f( size.x,  size.y, 0.0f), sp::Vector2f(1, 0));
    
    return std::make_shared<MeshData>(vertices);
}

std::shared_ptr<MeshData> MeshData::createDoubleSidedQuad(sp::Vector2f size)
{
    size *= 0.5f;
    
    std::vector<Vertex> vertices;
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
    
    return std::make_shared<MeshData>(vertices);
}

}//!namespace sp
