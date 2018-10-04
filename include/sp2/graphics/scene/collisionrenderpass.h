#ifndef SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
#define SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>

namespace sp {
class Scene;
class Node;
class Camera;
class CollisionRenderPass : public RenderPass
{
public:
    CollisionRenderPass();
    CollisionRenderPass(P<Camera> camera);

    virtual void render(RenderQueue& queue) override;

    void setScene(P<Scene> scene);
    void setCamera(P<Camera> camera);
private:
    bool enabled;
    bool enabled_toggled;
    P<Camera> specific_camera;
    
    void renderScene(RenderQueue& queue, Scene* scene);

    std::shared_ptr<MeshData> mesh;
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
