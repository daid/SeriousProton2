#ifndef SP2_GRAPHICS_GUI_GRAPHICSLAYER_H
#define SP2_GRAPHICS_GUI_GRAPHICSLAYER_H

#include <sp2/graphics/graphicslayer.h>

namespace sp {
namespace gui {

class Widget;
class GraphicsLayer : public sp::GraphicsLayer
{
public:
    GraphicsLayer(int priority);
    
    void setFixedVirtualSize(sf::Vector2f size);
    void setMinimalVirtualSize(sf::Vector2f size);
    void setMaximumVirtualSize(sf::Vector2f size);
    virtual void render(sf::RenderTarget& window);
private:
    sf::Vector2f min_size;
    sf::Vector2f max_size;
    P<Widget> root;
    
    static P<GraphicsLayer> default_gui_layer;
    
    friend class Widget;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_CONTAINER_H
