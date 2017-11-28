#ifndef SP2_GRAPHICS_MESHDATA_H
#define SP2_GRAPHICS_MESHDATA_H

#include <SFML/System/NonCopyable.hpp>
#include "sp2/math/vector.h"
#include "sp2/string.h"
#include <memory>

namespace sp {

class MeshData : public sf::NonCopyable
{
public:
    class Vertex
    {
    public:
        Vertex() {}
        
        Vertex(sp::Vector3f pos)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
            uv[0] = 0.0; uv[1] = 0.0;
        }
        Vertex(sp::Vector3f pos, sp::Vector2f uv)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
            this->uv[0] = uv.x; this->uv[1] = uv.y;
        }
        Vertex(sp::Vector3f pos, sp::Vector3f norm, sp::Vector2f uv)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = norm.x; normal[1] = norm.y; normal[2] = norm.z;
            this->uv[0] = uv.x; this->uv[1] = uv.y;
        }

        float position[3];
        float normal[3];
        float uv[2];
    };

    MeshData(const std::vector<Vertex>& vertices);
    ~MeshData();
    
    void render();
    
    static std::shared_ptr<MeshData> create(const std::vector<Vertex>& vertices);
    static std::shared_ptr<MeshData> createQuad(sp::Vector2f size);
    static std::shared_ptr<MeshData> createDoubleSidedQuad(sp::Vector2f size);
private:

    int vertex_count;
    Vertex* vertices;
    unsigned int vbo;

    MeshData();
};

}//!namespace sp

#endif//SP2_GRAPHICS_MESHDATA_H
