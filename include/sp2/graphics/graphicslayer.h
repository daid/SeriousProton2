#ifndef SP2_GRAPHICS_GRAPHICSLAYER_H
#define SP2_GRAPHICS_GRAPHICSLAYER_H

#include <sp2/pointerVector.h>
#include <SFML/Graphics/RenderTarget.hpp>

namespace sp {

class GraphicsLayer : public AutoPointerObject
{
public:
    GraphicsLayer(int priority);

    void enable();
    void disable();
    
    void setPriority(int priority);
    
    virtual void render(sf::RenderTarget& window) = 0;
private:
    bool enabled;
    int priority;
    
    static void sortLayers();
    static PVector<GraphicsLayer> layers;
    
    friend class Window;
};

};//!namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
