#ifndef SP2_GRAPHICS_MESH_OBJ_H
#define SP2_GRAPHICS_MESH_OBJ_H

#include <sp2/graphics/meshdata.h>
#include <sp2/math/quaternion.h>
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
    class Point
    {
    public:
        string name;
        Vector3d position;
        Quaterniond rotation;
    };

    void setMode(Mode mode) { ObjLoader::mode = mode; }
    Texture* getTextureFor(const string& name);
    const std::vector<Point>& getPoints(const string& name);
protected:
    virtual std::shared_ptr<MeshData> load(const string& name) override;

private:
    class Info
    {
    public:
        Texture* texture = nullptr;
        std::vector<Point> points;
    };
    Mode mode;
    std::unordered_map<string, Info> obj_info;
};

extern ObjLoader obj_loader;

}//namespace sp

#endif//SP2_GRAPHICS_MESH_OBJ_H
