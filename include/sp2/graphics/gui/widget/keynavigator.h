#ifndef SP2_GRAPHICS_GUI_KEYNAVIGATOR_H
#define SP2_GRAPHICS_GUI_KEYNAVIGATOR_H

#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/string.h>

namespace sp {
namespace io { class Keybinding; }
namespace gui {

class KeyNavigator : public Widget
{
public:
    KeyNavigator(P<Widget> parent);

    virtual void setAttribute(const string& key, const string& value) override;

    virtual void updateRenderData() override;
    virtual void onUpdate(float delta) override;
private:
    P<Widget> findFirstTarget(P<Widget> w);
    P<Widget> findNextTarget(P<Widget> w, P<Widget> after);
    bool canTarget(P<Widget> w);

    P<io::Keybinding> up;
    P<io::Keybinding> down;
    P<io::Keybinding> left;
    P<io::Keybinding> right;
    P<io::Keybinding> select;

    bool skip = true;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_KEYNAVIGATOR_H
