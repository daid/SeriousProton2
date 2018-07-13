#ifndef SP2_GRAPHICS_SCENE_RENDERPASS_H
#define SP2_GRAPHICS_SCENE_RENDERPASS_H

#include <sp2/pointer.h>
#include <sp2/io/pointer.h>
#include <sp2/math/vector2.h>
#include <map>

namespace sp {

class RenderPass : public AutoPointerObject
{
public:
    RenderPass();
    
    virtual void renderSetup(float aspect_ratio) = 0;
    virtual void renderExecute() = 0;

    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id);
    virtual void onPointerDrag(Vector2d position, int id);
    virtual void onPointerUp(Vector2d position, int id);
};

};//namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
