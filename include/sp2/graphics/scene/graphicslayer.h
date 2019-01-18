#ifndef SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H
#define SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H

#include <sp2/graphics/graphicslayer.h>
#include <sp2/pointerList.h>
#include <map>

namespace sp {

class RenderPass;
class SceneGraphicsLayer : public GraphicsLayer
{
public:
    SceneGraphicsLayer(int priority);
    virtual ~SceneGraphicsLayer();
    
    virtual void render(RenderQueue& queue) override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
    virtual void onTextInput(const string& text) override;
    virtual void onTextInput(TextInputEvent e) override;
    
    void addRenderPass(P<RenderPass> render_pass);
private:
    std::map<int, P<RenderPass>> pointer_render_pass;
    P<RenderPass> focus_render_pass;

    PList<RenderPass> render_passes;
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_GRAPHICSLAYER_H
