#ifndef SP2_GRAPHICS_MESH_OBJ_H
#define SP2_GRAPHICS_MESH_OBJ_H

#include <sp2/graphics/meshdata.h>
#include <sp2/io/directLoader.h>

namespace sp {

class Texture;
class ObjLoader : public io::DirectLoader<std::shared_ptr<MeshData>>
{
public:
    enum class Mode
    {
        Normal,
        DiffuseMaterialColorToNormal,
        DiffuseMaterialColorToTexture,
    };

    void setMode(Mode mode) { ObjLoader::mode = mode; }
    Texture* getTextureFor(string name);
protected:
    virtual std::shared_ptr<MeshData> load(string name) override;

private:
    Mode mode;
    std::unordered_map<string, Texture*> texture_cache;
};

extern ObjLoader obj_loader;

}//namespace sp

#endif//SP2_GRAPHICS_MESH_OBJ_H
