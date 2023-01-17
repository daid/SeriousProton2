#ifndef SP2_GRAPHICS_SCENE_RENDERPASS_H
#define SP2_GRAPHICS_SCENE_RENDERPASS_H

#include <sp2/pointer.h>
#include <sp2/io/pointer.h>
#include <sp2/io/textinput.h>
#include <sp2/math/vector2.h>
#include <map>

namespace sp {

class RenderQueue;
class RenderPass : public AutoPointerObject
{
public:
    RenderPass();
    
    virtual void render(RenderQueue& queue) = 0;

    virtual bool onPointerMove(Vector2d position, int id);
    virtual void onPointerLeave(int id);
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id);
    virtual void onPointerDrag(Vector2d position, int id);
    virtual void onPointerUp(Vector2d position, int id);
    virtual bool onWheelMove(Vector2d position, io::Pointer::Wheel direction);
    virtual void onTextInput(const string& text);
    virtual void onTextInput(TextInputEvent e);
};

}//namespace sp

#endif//SP2_GRAPHICS_SCENE_RENDERPASS_H
