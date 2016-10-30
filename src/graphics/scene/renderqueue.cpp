#include <sp2/logging.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/meshdata.h>
#include <SFML/Graphics/Shader.hpp>

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
        sf::Shader::bind(item.data.shader);
        item.data.shader->setUniform("projection_matrix", projection);
        item.data.shader->setUniform("camera_matrix", camera_transform);
        item.data.shader->setUniform("object_matrix", item.transform);
        item.data.mesh->render();
    }
}

};//!namespace sp
