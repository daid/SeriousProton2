#ifndef SP2_GRAPHICS_SCENE_RENDERPASS_H
#define SP2_GRAPHICS_SCENE_RENDERPASS_H

#include <sp2/graphics/graphicslayer.h>
#include <map>

namespace sp {

class RenderPass : public AutoPointerObject
{
public:
    RenderPass(string target_layer);
    
    virtual void render(P<GraphicsLayer> layer, float aspect_ratio) = 0;

    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id);
    virtual void onPointerDrag(Vector2d position, int id);
    virtual void onPointerUp(Vector2d position, int id);
    
    string getTargetLayer() const;
    virtual std::vector<string> getSourceLayers() const;
private:
    string target_layer;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
