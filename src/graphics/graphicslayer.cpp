#include <sp2/graphics/graphicslayer.h>

namespace sp {

PList<GraphicsLayer> GraphicsLayer::layers;

GraphicsLayer::GraphicsLayer(int priority)
{
    enabled = true;
    layers.add(this);
    setPriority(priority);
    viewport = sf::FloatRect(0, 0, 1, 1);
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
    layers.sort([](const P<GraphicsLayer>& a, const P<GraphicsLayer>& b){
        return a->priority - b->priority;
    });
}

Vector2d GraphicsLayer::screenToViewportPosition(Vector2d position)
{
    //Position is now in the range -1,-1 to 1,1. Where -1,-1 is the bottom left corner of the screen and 1,1 the top right.
    //The viewport is defined in the range 0,0 to 1,1 in the same coordinates.
    position.x = ((position.x + 1.0) / viewport.width) - 1.0 - viewport.left * 2.0 / viewport.width;
    position.y = ((position.y + 1.0) / viewport.height) - 1.0 - viewport.top * 2.0 / viewport.height;
    return position;
}

};//!namespace sp
