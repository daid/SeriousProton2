#ifndef SP2_GRAPHICS_GUI_BUTTON_H
#define SP2_GRAPHICS_GUI_BUTTON_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Label : public Widget
{
public:
    Label(P<Container> parent);

    void setLabel(string label);
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void render(sf::RenderTarget& window) override;
private:
    Alignment text_alignment;
    string label;
    bool vertical;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H

