#ifndef SP2_GRAPHICS_MESH_OBJ_H
#define SP2_GRAPHICS_MESH_OBJ_H

#include <sp2/graphics/meshdata.h>

namespace sp {

class ObjLoader : NonCopyable
{
public:
    static std::shared_ptr<MeshData> load(string resource_name);
};

}//namespace sp

#endif//SP2_GRAPHICS_MESH_OBJ_H
