#ifndef SP2_GRAPHICS_GUI_SLIDER_H
#define SP2_GRAPHICS_GUI_SLIDER_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class ThemeImage;
class Slider : public Widget
{
public:
    Slider(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerDrag(Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
private:
    float min_value, max_value;
    float value;
    P<ThemeImage> dial;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_SLIDER_H

