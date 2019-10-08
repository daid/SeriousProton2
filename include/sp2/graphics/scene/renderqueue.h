#ifndef SP2_GRAPHICS_SCENE_RENDERQUEUE_H
#define SP2_GRAPHICS_SCENE_RENDERQUEUE_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/math/matrix4x4.h>
#include <sp2/pointer.h>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>


namespace sp {

class Camera;
class RenderQueue
{
public:
    RenderQueue();

    void setTargetAspectSize(float aspect_ratio);
    void setAspectRatio(float aspect_ratio);
    void setCamera(P<Camera> camera);
    void setCamera(const Matrix4x4f& camera_projection, const Matrix4x4f& camera_transform);
    void add(std::function<void()> function);
    void add(const Matrix4x4f& transform, const RenderData& data);
    void render();
private:
    class Item
    {
    public:
        enum class Type
        {
            CameraProjection,
            CameraTransform,
            RenderItem,
            FunctionCall,
        };
    
        Item(Type type, const Matrix4x4f& transform)
        : type(type), transform(transform)
        {
        }

        Item(const Matrix4x4f& transform, const RenderData& data)
        : type(Type::RenderItem), transform(transform), data(data)
        {
        }

        Item(std::function<void()> function)
        : type(Type::FunctionCall), function(function)
        {
        }

        bool operator<(const Item& other) const
        {
            return data < other.data;
        }

        Type type;
        Matrix4x4f transform;
        RenderData data;
        std::function<void()> function;
    };

    void render(std::vector<Item>& list);

    Matrix4x4f camera_projection;
    Matrix4x4f camera_transform;
    std::vector<Item> render_list;
    int render_list_sort_start;
    
    float target_aspect_ratio;
    float aspect_ratio;

#ifdef SP2_USE_RENDER_THREAD
    void renderThread();

    std::thread render_thread;
    std::mutex render_mutex;
    std::condition_variable render_trigger;
    bool render_data_ready;
    std::vector<Item> ready_render_list;
#endif
};

}//namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
