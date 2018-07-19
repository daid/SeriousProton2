#ifndef SP2_GRAPHICS_GRAPHICSLAYER_H
#define SP2_GRAPHICS_GRAPHICSLAYER_H

#include <sp2/pointerList.h>
#include <sp2/io/pointer.h>
#include <sp2/math/vector.h>
#include <sp2/math/rect.h>
#include <SFML/Graphics/RenderTarget.hpp>

namespace sp {

class Window;
class RenderTexture;
class RenderQueue;
class GraphicsLayer : public AutoPointerObject
{
public:
    GraphicsLayer(int priority);

    void enable();
    void disable();
    bool isEnabled();
    
    void setPriority(int priority);
    void setTarget(RenderTexture* render_texture);
    RenderTexture* getTarget();
    
    virtual void render(RenderQueue& queue) = 0;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) = 0;
    virtual void onPointerDrag(Vector2d position, int id) = 0;
    virtual void onPointerUp(Vector2d position, int id) = 0;

    void setViewport(Rect2d rect) { viewport = rect; }
protected:
    Vector2d screenToViewportPosition(Vector2d position);

    Rect2d viewport;
private:
    bool enabled;
    int priority;
    RenderTexture* render_texture;
    
    friend class Window;
    friend class Engine;
};

};//namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
