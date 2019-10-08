#include <sp2/graphics/graphicslayer.h>
#include <sp2/window.h>
#include <sp2/assert.h>

namespace sp {

GraphicsLayer::GraphicsLayer(int priority)
{
    enabled = true;
    setPriority(priority);
    render_texture = nullptr;
    viewport = Rect2d(0, 0, 1, 1);
}

void GraphicsLayer::enable()
{
    enabled = true;
}

void GraphicsLayer::disable()
{
    enabled = false;
}

bool GraphicsLayer::isEnabled()
{
    return enabled;
}

void GraphicsLayer::setPriority(int priority)
{
    this->priority = priority;
}

void GraphicsLayer::setTarget(RenderTexture* render_texture)
{
    this->render_texture = render_texture;
}

RenderTexture* GraphicsLayer::getTarget()
{
    return render_texture;
}

Vector2d GraphicsLayer::screenToViewportPosition(Vector2d position)
{
    //Position is now in the range -1,-1 to 1,1. Where -1,-1 is the bottom left corner of the screen and 1,1 the top right.
    //The viewport is defined in the range 0,0 to 1,1 in the same coordinates.
    position.x = ((position.x + 1.0) / viewport.size.x) - 1.0 - viewport.position.x * 2.0 / viewport.size.x;
    position.y = ((position.y + 1.0) / viewport.size.y) - 1.0 - viewport.position.y * 2.0 / viewport.size.y;
    return position;
}

}//namespace sp
