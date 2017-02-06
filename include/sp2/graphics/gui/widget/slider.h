#ifndef SP2_GRAPHICS_GUI_SLIDER_H
#define SP2_GRAPHICS_GUI_SLIDER_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Slider : public Widget
{
public:
    Slider(P<Container> parent);

    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void render(sf::RenderTarget& window) override;
    virtual bool onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id) override;
    virtual void onPointerDrag(sf::Vector2f position, int id) override;
    virtual void onPointerUp(sf::Vector2f position, int id) override;
private:
    float min_value, max_value;
    float value;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_SLIDER_H

