#include <sp2/logging.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/shader.h>
#include <sp2/graphics/opengl.h>

namespace sp {

void RenderQueue::clear()
{
    render_list.clear();
}

void RenderQueue::add(const Matrix4x4d& transform, const RenderData& data)
{
    if (!data.shader)
        return;
    render_list.emplace_back(transform, data);
}

void RenderQueue::render(const Matrix4x4d& projection, const Matrix4x4d& camera_transform)
{
    std::sort(render_list.begin(), render_list.end());
    for(Item& item : render_list)
    {
        switch(item.data.type)
        {
        case RenderData::Type::None:
        case RenderData::Type::Normal:
        case RenderData::Type::Transparent:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        case RenderData::Type::Additive:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;
        }
        if (item.data.type == RenderData::Type::Transparent || item.data.type == RenderData::Type::Additive)
            glDepthMask(false);
        item.data.shader->setUniform("projection_matrix", projection);
        item.data.shader->setUniform("camera_matrix", camera_transform);
        item.data.shader->setUniform("object_matrix", item.transform);
        item.data.shader->setUniform("object_scale", sf::Vector3f(item.data.scale.x, item.data.scale.y, item.data.scale.z));
        item.data.shader->setUniform("color", sf::Glsl::Vec4(item.data.color.r, item.data.color.g, item.data.color.b, item.data.color.a));
        if (item.data.texture)
            item.data.shader->setUniform("texture_map", *item.data.texture->get());
        sf::Shader::bind(item.data.shader);
        item.data.mesh->render();
        if (item.data.type == RenderData::Type::Transparent || item.data.type == RenderData::Type::Additive)
            glDepthMask(true);
    }
}

};//namespace sp
