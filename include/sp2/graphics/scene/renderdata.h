#ifndef SP2_GRAPHICS_SCENE_RENDERDATA_H
#define SP2_GRAPHICS_SCENE_RENDERDATA_H

#include <sp2/string.h>
#include <sp2/graphics/shader.h>
#include <sp2/graphics/color.h>
#include <sp2/graphics/texture.h>
#include <vector>
#include <memory>

namespace sp {

class MeshData;
class RenderData
{
public:
    enum class Type
    {
        None,       //Default rendering action: No rendering at all.
        Normal,     //Rendered with full depth testing and depth drawing
        Transparent,//Rendered with depth testing, but without drawing to the depth buffer.
        Additive,   //Rendered with additive blending and without depth buffer drawing.
    };
    int order;
    Type type;
    Shader* shader;
    std::shared_ptr<MeshData> mesh;
    Color color;
    Texture* texture;
    Vector3f scale;
    
    RenderData();
    
    bool operator<(const RenderData& data) const;
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERDATA_H
