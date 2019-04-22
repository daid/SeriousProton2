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
        : position(pos)
        {
        }
        Vertex(Vector3f pos, Vector2f uv)
        : position(pos), uv(uv)
        {
        }
        Vertex(Vector3f pos, Vector3f norm)
        : position(pos), normal(norm)
        {
        }
        Vertex(Vector3f pos, Vector3f norm, Vector2f uv)
        : position(pos), normal(norm), uv(uv)
        {
        }
        
        Vector3f position;
        Vector3f normal;
        Vector2f uv;
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
    static std::shared_ptr<MeshData> createCircle(float radius, int point_count);
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
