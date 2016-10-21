#ifndef SP2_GRAPHICS_GUI_BUTTON_H
#define SP2_GRAPHICS_GUI_BUTTON_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Button : public Widget
{
public:
    Button(P<Container> parent);

    void setLabel(string label);
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void render(sf::RenderTarget& window) override;
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) override;
    virtual void onPointerUp(sf::Vector2f position, int id) override;
private:
    string label;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_WIDGET_H

