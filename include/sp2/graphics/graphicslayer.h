#ifndef SP2_GRAPHICS_GRAPHICSLAYER_H
#define SP2_GRAPHICS_GRAPHICSLAYER_H

#include <sp2/pointerVector.h>
#include <sp2/io/pointer.h>
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
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) = 0;
    virtual void onPointerDrag(sf::Vector2f position, int id) = 0;
    virtual void onPointerUp(sf::Vector2f position, int id) = 0;

    void setViewport(sf::FloatRect rect) { viewport = rect; }
protected:
    sf::FloatRect viewport;
private:
    bool enabled;
    int priority;
    
    static void sortLayers();
    static PVector<GraphicsLayer> layers;

    friend class Window;
    friend class Engine;
};

};//!namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
