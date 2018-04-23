#ifndef SP2_GRAPHICS_GUI_BUTTON_H
#define SP2_GRAPHICS_GUI_BUTTON_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace gui {

class Label;
class Button : public Widget
{
public:
    Button(P<Widget> parent);

    void setLabel(string label);
    virtual void setAttribute(const string& key, const string& value) override;
    
    virtual void updateRenderData() override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
private:
    P<Label> label;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_BUTTON_H

