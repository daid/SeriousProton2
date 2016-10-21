#ifndef SP2_GRAPHICS_GUI_PANEL_H
#define SP2_GRAPHICS_GUI_PANEL_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class Panel : public Widget
{
public:
    Panel(P<Container> parent);
    
    virtual void render(sf::RenderTarget& window) override;
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) override;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_PANEL_H

