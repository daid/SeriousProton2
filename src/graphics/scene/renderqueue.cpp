#include <sp2/logging.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/shader.h>
#include <sp2/graphics/opengl.h>
#include <sp2/scene/camera.h>


namespace sp {

RenderQueue::RenderQueue()
{
    render_list.clear();
    render_list_sort_start = 0;
    
#ifdef SP2_USE_RENDER_THREAD
    render_data_ready = false;
    std::thread thread ([this](){renderThread();});
    std::swap(render_thread, thread);
#endif
}

void RenderQueue::setTargetAspectSize(float aspect_ratio)
{
    this->target_aspect_ratio = aspect_ratio;
}

void RenderQueue::setAspectRatio(float aspect_ratio)
{
    this->aspect_ratio = aspect_ratio;
}

void RenderQueue::setCamera(P<Camera> camera)
{
    camera->setAspectRatio(target_aspect_ratio * aspect_ratio);
    setCamera(camera->getProjectionMatrix(), camera->getGlobalTransform().inverse());
}

void RenderQueue::setCamera(const Matrix4x4d& camera_projection, const Matrix4x4d& camera_transform)
{
    std::sort(render_list.begin() + render_list_sort_start, render_list.end());
    render_list.emplace_back(Item::Type::CameraProjection, camera_projection);
    render_list.emplace_back(Item::Type::CameraTransform, camera_transform);
    render_list_sort_start = render_list.size();
}

void RenderQueue::add(std::function<void()> function)
{
    std::sort(render_list.begin() + render_list_sort_start, render_list.end());
    render_list.emplace_back(function);
    render_list_sort_start = render_list.size();
}

void RenderQueue::add(const Matrix4x4d& transform, const RenderData& data)
{
    if (!data.shader)
        return;
    render_list.emplace_back(transform, data);
}

void RenderQueue::render()
{
    std::sort(render_list.begin() + render_list_sort_start, render_list.end());
    render_list_sort_start = 0;
#ifdef SP2_USE_RENDER_THREAD
    {
        std::unique_lock<std::mutex> lock(render_mutex);
        if (render_data_ready)
            render_trigger.wait(lock, [this](){ return !render_data_ready; });
        std::swap(ready_render_list, render_list);
        render_data_ready = true;
    }
    render_trigger.notify_one();
#else
    render(render_list);
#endif
}

#ifdef SP2_USE_RENDER_THREAD
void RenderQueue::renderThread()
{
    while(true)
    {
        {
            std::unique_lock<std::mutex> lock(render_mutex);
            if (!render_data_ready)
                render_trigger.wait(lock, [this](){ return render_data_ready; });
        }
        render(ready_render_list);
        {
            std::unique_lock<std::mutex> lock(render_mutex);
            render_data_ready = false;
        }
        render_trigger.notify_one();
    }
}
#endif

void RenderQueue::render(std::vector<Item>& list)
{
    for(Item& item : list)
    {
        switch(item.type)
        {
        case Item::Type::CameraProjection:
            camera_projection = item.transform;
            break;
        case Item::Type::CameraTransform:
            camera_transform = item.transform;
            break;
        case Item::Type::FunctionCall:
            item.function();
            break;
        case Item::Type::RenderItem:
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
            item.data.shader->setUniform("projection_matrix", camera_projection);
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
            break;
        }
    }
    list.clear();
}

};//namespace sp
