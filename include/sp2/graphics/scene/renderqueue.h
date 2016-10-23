#ifndef SP2_GRAPHICS_SCENE_RENDERQUEUE_H
#define SP2_GRAPHICS_SCENE_RENDERQUEUE_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/math/matrix4x4.h>
#include <SFML/Graphics/RenderTarget.hpp>
#include <vector>

namespace sp {

class RenderQueue
{
public:
    void clear();
    void add(const Matrix4x4d& transform, const RenderData& data);
    void render(const Matrix4x4d& projection, const Matrix4x4d& camera_transform, sf::RenderTarget& target);
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

    std::vector<Item> render_list;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
