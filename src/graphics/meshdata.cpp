#include <sp2/graphics/opengl.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/logging.h>
#include <limits>

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

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

}//!namespace sp
