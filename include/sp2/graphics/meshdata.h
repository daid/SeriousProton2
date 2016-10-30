#ifndef SP2_GRAPHICS_MESHDATA_H
#define SP2_GRAPHICS_MESHDATA_H

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Vector3.hpp>
#include "sp2/string.h"

namespace sp {

class MeshData : public sf::NonCopyable
{
public:
    class Vertex
    {
    public:
        Vertex() {}
        
        Vertex(sf::Vector3f pos)
        {
            position[0] = pos.x; position[1] = pos.y; position[2] = pos.z;
            normal[0] = 0.0; normal[1] = 0.0; normal[2] = 0.0;
            uv[0] = 0.0; uv[1] = 0.0;
        }

        float position[3];
        float normal[3];
        float uv[2];
    };

    MeshData(std::vector<Vertex>& vertices);
    ~MeshData();
    
    void render();
private:
    int vertex_count;
    Vertex* vertices;
    unsigned int vbo;

    MeshData();
};

}//!namespace sp

#endif//SP2_GRAPHICS_MESHDATA_H
