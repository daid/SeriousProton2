#ifndef SP2_GRAPHICS_GUI_GRAPHICSLAYER_H
#define SP2_GRAPHICS_GUI_GRAPHICSLAYER_H

#include <sp2/graphics/graphicslayer.h>
#include <map>

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
    virtual void render(sf::RenderTarget& window) override;
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) override;
    virtual void onPointerDrag(sf::Vector2f position, int id) override;
    virtual void onPointerUp(sf::Vector2f position, int id) override;
    P<Widget> getRoot() { return root; }
    
    sf::Vector2f virtualPositionToScreen(sf::Vector2f position);
    sf::Vector2f screenToVirtualPosition(sf::Vector2f position);
private:
    sf::Vector2f min_size;
    sf::Vector2f max_size;
    P<Widget> root;
    P<Widget> focus_widget;
    std::map<int, P<Widget>> pointer_widget;
    //transform to translate 0.0-1.0 screen coordinates to virtual window coordinates of the GUI
    sf::Transform virtual_position_transform;
    
    bool draw_debug;
    
    static P<GraphicsLayer> default_gui_layer;
    
    void drawWidgets(sf::RenderTarget& window, P<Widget> w);
    P<Widget> widgetAtPosition(P<Widget> w, sf::Vector2f position);
    
    void debugDump(P<Widget> widget, int indent);
    void debugDraw(sf::RenderTarget& window, P<Widget> widget);
    
    friend class Widget;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_GRAPHICSLAYER_H
