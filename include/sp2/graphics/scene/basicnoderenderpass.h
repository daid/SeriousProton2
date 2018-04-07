#ifndef SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
#define SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <list>

namespace sp {
class Scene;
class Node;
class Camera;
class BasicNodeRenderPass : public RenderPass
{
public:
    BasicNodeRenderPass();
    BasicNodeRenderPass(P<Scene> scene);
    BasicNodeRenderPass(P<Camera> camera);
    
    virtual void render(P<GraphicsLayer> layer, float aspect_ratio) override;

    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    
    void addScene(P<Scene> scene, P<Camera> camera=nullptr);
protected:
    virtual void addNodeToRenderQueue(Node* node);
    
    RenderQueue queue;
private:
    class SceneWithCamera
    {
    public:
        P<Scene> scene;
        P<Camera> camera;
    };
    std::list<SceneWithCamera> scenes;
    std::map<int, P<Scene>> pointer_scene;
    std::map<int, P<Camera>> pointer_camera;
    
    bool privateOnPointerDown(P<Scene> scene, P<Camera> camera, io::Pointer::Button button, Vector2d position, int id);
    void renderScene(P<Scene> scene, P<Camera> camera, P<GraphicsLayer> layer, float aspect_ratio);
    void recursiveNodeRender(Node* node);
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
