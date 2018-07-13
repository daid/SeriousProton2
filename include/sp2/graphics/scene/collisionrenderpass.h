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
    CollisionRenderPass(P<Scene> scene);
    CollisionRenderPass(P<Scene> scene, P<Camera> camera);
    
    virtual void renderSetup(float aspect_ratio) override;
    virtual void renderExecute() override;

    void setScene(P<Scene> scene);
    void setCamera(P<Camera> camera);
private:
    bool enabled;
    bool enabled_toggled;
    P<Scene> single_scene;
    P<Camera> specific_camera;
    
    void setupScene(Scene* scene, float aspect_ratio);

    std::shared_ptr<MeshData> mesh;
    std::vector<RenderQueue> queues;
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
