#include <sp2/graphics/graphicslayer.h>

namespace sp {

PVector<GraphicsLayer> GraphicsLayer::layers;

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

};//!namespace sp
