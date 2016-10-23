#include <sp2/graphics/scene/renderqueue.h>

namespace sp {

void RenderQueue::clear()
{
    render_list.clear();
}

void RenderQueue::add(const Matrix4x4d& transform, const RenderData& data)
{
    render_list.emplace_back(transform, data);
}

void RenderQueue::render(const Matrix4x4d& projection, const Matrix4x4d& camera_transform, sf::RenderTarget& target)
{
    std::sort(render_list.begin(), render_list.end());
    for(Item& item : render_list)
    {
        
    }
}

};//!namespace sp
