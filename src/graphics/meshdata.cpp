#include <sp2/graphics/opengl.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/shader.h>
#include <sp2/logging.h>
#include <limits>
#include <string.h>
#include <stddef.h>

const unsigned int NO_BUFFER = std::numeric_limits<unsigned int>::max();

namespace sp {

MeshData::MeshData(Type type)
: type(type)
{
    vertices_vbo = NO_BUFFER;
    indices_vbo = NO_BUFFER;
    dirty = true;
    revision = 0;
}

MeshData::MeshData(Vertices&& vertices, Indices&& indices, Type type)
: MeshData(type)
{
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
}

MeshData::~MeshData()
{
    if (vertices_vbo != NO_BUFFER)
        glDeleteBuffers(1, &vertices_vbo);
    if (indices_vbo != NO_BUFFER)
        glDeleteBuffers(1, &indices_vbo);
}

void MeshData::render()
{
    if (vertices.size() < 1)
        return;
    if (vertices_vbo == NO_BUFFER)
    {
        glGenBuffers(1, &vertices_vbo);
        glGenBuffers(1, &indices_vbo);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
    if (dirty)
    {
        int gl_type = GL_STATIC_DRAW;
        if (type == Type::Dynamic)
            gl_type = GL_DYNAMIC_DRAW;

        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), gl_type);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), gl_type);
        dirty = false;
    }

    if (Shader::bound_shader)
    {
        if (Shader::bound_shader->vertex_attribute != -1)
            glVertexAttribPointer(Shader::bound_shader->vertex_attribute, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
        if (Shader::bound_shader->normal_attribute != -1)
            glVertexAttribPointer(Shader::bound_shader->normal_attribute, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        if (Shader::bound_shader->uv_attribute != -1)
            glVertexAttribPointer(Shader::bound_shader->uv_attribute, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    }

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, nullptr);
}

void MeshData::update(Vertices&& vertices, Indices&& indices)
{
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
    dirty = true;
    revision++;
}

std::shared_ptr<MeshData> MeshData::create(Vertices&& vertices, Indices&& indices, Type type)
{
    return std::make_shared<MeshData>(std::forward<Vertices>(vertices), std::forward<Indices>(indices), type);
}

std::shared_ptr<MeshData> MeshData::createQuad(Vector2f size, Vector2f uv0, Vector2f uv1)
{
    size *= 0.5f;
    
    Vertices vertices;
    Indices indices{0,1,2,2,1,3};
    vertices.reserve(4);
    
    vertices.emplace_back(Vector3f(-size.x, -size.y, 0.0f), Vector2f(uv0.x, uv1.y));
    vertices.emplace_back(Vector3f( size.x, -size.y, 0.0f), Vector2f(uv1.x, uv1.y));
    vertices.emplace_back(Vector3f(-size.x,  size.y, 0.0f), Vector2f(uv0.x, uv0.y));
    vertices.emplace_back(Vector3f( size.x,  size.y, 0.0f), Vector2f(uv1.x, uv0.y));
    
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

std::shared_ptr<MeshData> MeshData::createDoubleSidedQuad(Vector2f size, Vector2f uv0, Vector2f uv1)
{
    size *= 0.5f;
    
    Vertices vertices;
    Indices indices{0,1,2,2,1,3, 0,2,1,2,3,1};
    vertices.emplace_back(Vector3f(-size.x, -size.y, 0.0f), Vector2f(uv0.x, uv1.y));
    vertices.emplace_back(Vector3f( size.x, -size.y, 0.0f), Vector2f(uv1.x, uv1.y));
    vertices.emplace_back(Vector3f(-size.x,  size.y, 0.0f), Vector2f(uv0.x, uv0.y));
    vertices.emplace_back(Vector3f( size.x,  size.y, 0.0f), Vector2f(uv1.x, uv0.y));
    
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

std::shared_ptr<MeshData> MeshData::createCircle(float radius, int point_count)
{
    Vertices vertices;
    vertices.reserve(point_count + 1);
    Indices indices;
    indices.reserve(point_count * 3);
    vertices.emplace_back(Vector3f(0, 0, 0), Vector2f(0, 0));
    for(int n=0; n<point_count; n++)
    {
        float f = float(n) / float(point_count);
        vertices.emplace_back(Vector3f(std::sin(f * sp::pi * 2) * radius, std::cos(f * sp::pi * 2) * radius, 0.0f), Vector2f(f, 1));
        
        indices.push_back(0);
        indices.push_back(((n + 1) % point_count) + 1);
        indices.push_back(n + 1);
    }
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

}//namespace sp
