#ifndef SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
#define SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H

#include <sp2/graphics/scene/renderpass.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/math/ray.h>
#include <sp2/pointerList.h>
#include <list>

namespace sp {
class Scene;
class Node;
class Camera;
class BasicNodeRenderPass : public RenderPass
{
public:
    BasicNodeRenderPass();
    BasicNodeRenderPass(P<Camera> camera);
    
    virtual void render(RenderQueue& queue) override;

    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;
    
    void addCamera(P<Camera> camera);
protected:
    virtual void addNodeToRenderQueue(RenderQueue& queue, P<Node>& node);

    PList<Camera> cameras;
private:
    std::map<int, P<Scene>> pointer_scene;
    std::map<int, P<Camera>> pointer_camera;
    P<Scene> focus_scene;
    
    bool privateOnPointerDown(P<Scene> scene, P<Camera> camera, io::Pointer::Button button, Vector2d position, int id);
    void renderScene(RenderQueue& queue, P<Scene> scene, P<Camera> camera);
    void recursiveNodeRender(RenderQueue& queue, P<Node> node);
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_BASICNODERENDERPASS_H
