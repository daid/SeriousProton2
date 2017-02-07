#ifndef SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H
#define SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>

namespace sp {
class Scene;
class SceneNode;
class CameraNode;
class CollisionRenderPass : public RenderPass
{
public:
    CollisionRenderPass(string target_layer);
    CollisionRenderPass(string target_layer, P<Scene> scene);
    CollisionRenderPass(string target_layer, P<Scene> scene, P<CameraNode> camera);
    
    virtual void render(sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio) override;

    void setScene(P<Scene> scene);
    void setCamera(P<CameraNode> camera);
private:
    P<Scene> single_scene;
    P<CameraNode> specific_camera;
    
    void renderScene(Scene* scene, sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio);

    RenderQueue queue;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_COLLISIONRENDERPASS_H