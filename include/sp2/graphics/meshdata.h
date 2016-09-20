#ifndef SP2_GRAPHICS_MESHDATA_H
#define SP2_GRAPHICS_MESHDATA_H

#include <SFML/System/NonCopyable.hpp>
#include "sp2/string.h"

class MeshData : public sf::NonCopyable
{
public:
    class Vertex
    {
    public:
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

#endif//SP2_GRAPHICS_MESHDATA_H
