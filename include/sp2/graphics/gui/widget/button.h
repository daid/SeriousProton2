#ifndef SP2_GRAPHICS_GUI_BUTTON_H
#define SP2_GRAPHICS_GUI_BUTTON_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>
#include <functional>

namespace sp {
namespace gui {

class Button : public Widget
{
public:
    Button(P<Container> parent);

    void setLabel(string label);
    
    virtual void render(sf::RenderTarget& window) override;
private:
    string label;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H

