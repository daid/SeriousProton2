#ifndef SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
#define SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <map>

namespace sp {
class Scene;
class Node;
class Camera;
class BasicNodeRenderPass : public RenderPass
{
public:
    BasicNodeRenderPass(string target_layer);
    BasicNodeRenderPass(string target_layer, P<Scene> scene);
    BasicNodeRenderPass(string target_layer, P<Scene> scene, P<Camera> camera);
    
    virtual void render(sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio) override;
    
    void setScene(P<Scene> scene);
    void setCamera(P<Camera> camera);
private:
    P<Scene> single_scene;
    P<Camera> specific_camera;
    
    void renderScene(Scene* scene, sf::RenderTarget& target, P<GraphicsLayer> layer, float aspect_ratio);
    
    RenderQueue queue;
    void recursiveNodeRender(Node* node);
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
