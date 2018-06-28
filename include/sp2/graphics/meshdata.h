#ifndef SP2_GRAPHICS_MESHDATA_H
#define SP2_GRAPHICS_MESHDATA_H

#include <sp2/nonCopyable.h>
#include <sp2/math/vector.h>
#include <sp2/string.h>
#include <memory>

namespace sp {

class MeshData : NonCopyable
{
public:
    enum class Type
    {
        Static,
        Dynamic
    };
    class Vertex
    {
    public:
        Vertex() {}
        
        Vertex(Vector3f pos)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
            uv[0] = 0.0; uv[1] = 0.0;
        }
        Vertex(Vector3f pos, Vector2f uv)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
            this->uv[0] = uv.x; this->uv[1] = uv.y;
        }
        Vertex(Vector3f pos, Vector3f norm, Vector2f uv)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = norm.x; normal[1] = norm.y; normal[2] = norm.z;
            this->uv[0] = uv.x; this->uv[1] = uv.y;
        }
        
        void setPosition(Vector3f pos)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
        }

        void setNormal(Vector3f norm)
        {
            normal[0] = norm.x; normal[1] = norm.y; normal[2] = norm.z;
        }

        void setUV(Vector2f uv)
        {
            this->uv[0] = uv.x; this->uv[1] = uv.y;
        }

        float position[3];
        float normal[3];
        float uv[2];
    };
    typedef std::vector<Vertex> Vertices;
    typedef std::vector<uint16_t> Indices;

    MeshData(Vertices&& vertices, Indices&& indices, Type type=Type::Static);
    ~MeshData();
    
    void render();
    void update(Vertices&& vertices, Indices&& indices);

    int getRevision() { return revision; }
    const Vertices& getVertices() { return vertices; }
    const Indices& getIndices() { return indices; }
    
    static std::shared_ptr<MeshData> create(Vertices&& vertices, Indices&& indices, Type type=Type::Static);
    static std::shared_ptr<MeshData> createQuad(Vector2f size, Vector2f uv0=Vector2f(0, 0), Vector2f uv1=Vector2f(1, 1));
    static std::shared_ptr<MeshData> createDoubleSidedQuad(Vector2f size, Vector2f uv0=Vector2f(0, 0), Vector2f uv1=Vector2f(1, 1));
private:
    Vertices vertices;
    Indices indices;
    unsigned int vertices_vbo;
    unsigned int indices_vbo;

    bool dirty;
    int revision;
    Type type;

    MeshData(Type type);
};

}//namespace sp

#endif//SP2_GRAPHICS_MESHDATA_H
