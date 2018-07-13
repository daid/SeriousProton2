#ifndef SP2_GRAPHICS_SCENE_RENDERQUEUE_H
#define SP2_GRAPHICS_SCENE_RENDERQUEUE_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/math/matrix4x4.h>
#include <vector>

namespace sp {

class RenderQueue
{
public:
    RenderQueue(const Matrix4x4d& camera_projection, const Matrix4x4d& camera_transform);

    void add(const Matrix4x4d& transform, const RenderData& data);
    void render();
private:
    class Item
    {
    public:
        Item(const Matrix4x4d& transform, const RenderData& data)
        : transform(transform), data(data)
        {
        }
        
        bool operator<(const Item& other) const
        {
            return data < other.data;
        }
        
        Matrix4x4d transform;
        RenderData data;
    };

    Matrix4x4d camera_projection;
    Matrix4x4d camera_transform;
    std::vector<Item> render_list;
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
