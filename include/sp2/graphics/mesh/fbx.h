#ifndef SP2_GRAPHICS_MESH_FBX_H
#define SP2_GRAPHICS_MESH_FBX_H

#include <sp2/graphics/meshdata.h>

namespace sp {

class FbxLoader : NonCopyable
{
public:
    static std::shared_ptr<MeshData> load(const string& resource_name);
};

}//namespace sp

#endif//SP2_GRAPHICS_MESHDATA_H
