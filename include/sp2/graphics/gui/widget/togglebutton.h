#ifndef SP2_GRAPHICS_GUI_TOGGLEBUTTON_H
#define SP2_GRAPHICS_GUI_TOGGLEBUTTON_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace io { class Keybinding; }
namespace gui {

class Label;
class ToggleButton : public Widget
{
public:
    ToggleButton(P<Widget> parent);

    void setLabel(const string& label);
    void setActive(bool active);
    bool isActive() { return active; }
    virtual void setAttribute(const string& key, const string& value) override;

    virtual void onUpdate(float delta) override;
    virtual void updateRenderData() override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
    virtual void onPointerUp(Vector2d position, int id) override;
private:
    bool keybinding_down = false;
    bool active = false;
    P<io::Keybinding> keybinding;
    P<Label> label;
    string style_name;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_TOGGLEBUTTON_H

