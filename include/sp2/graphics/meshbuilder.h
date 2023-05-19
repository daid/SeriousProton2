#ifndef SP2_GRAPHICS_MESHBUILDER_H
#define SP2_GRAPHICS_MESHBUILDER_H

#include <sp2/graphics/meshdata.h>

namespace sp {

class MeshBuilder : NonCopyable
{
public:
    void addQuad(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3);
    void addQuad(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f p3, Vector3f normal);

    std::shared_ptr<MeshData> create();
private:
    MeshData::Vertices vertices;
    MeshData::Indices indices;
};

}//namespace sp

#endif//SP2_GRAPHICS_MESHDATA_H
