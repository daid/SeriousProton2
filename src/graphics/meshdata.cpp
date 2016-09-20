#include <GL/glew.h>
#include <sp2/graphics/meshdata.h>
#include <limits>

const unsigned int NO_BUFFER = std::numeric_limits<unsigned int>::max();

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
#if FEATURE_3D_RENDERING
    if (!vertex_count)
        return;
    if (glGenBuffers)
    {
        if (vbo == NO_BUFFER)
        {
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(MeshVertex) * vertex_count, &vertices[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(float) * (3 * 2 + 2), (void*)offsetof(MeshVertex, position));
        glNormalPointer(GL_FLOAT, sizeof(float) * (3 * 2 + 2), (void*)offsetof(MeshVertex, normal));
        glTexCoordPointer(2, GL_FLOAT, sizeof(float) * (3 * 2 + 2), (void*)offsetof(MeshVertex, uv));
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    else
    {
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(float) * (3 * 2 + 2), &vertices[0].position[0]);
        glNormalPointer(GL_FLOAT, sizeof(float) * (3 * 2 + 2), &vertices[0].normal[0]);
        glTexCoordPointer(2, GL_FLOAT, sizeof(float) * (3 * 2 + 2), &vertices[0].uv[0]);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }
#endif//FEATURE_3D_RENDERING
}
