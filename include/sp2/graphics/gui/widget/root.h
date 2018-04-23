#ifndef SP2_GRAPHICS_GUI_ROOT_H
#define SP2_GRAPHICS_GUI_ROOT_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class RootWidget : public Widget
{
public:
    RootWidget(P<Node> parent, Vector2d size);
    
    virtual void onUpdate(float delta) override;
private:
    Vector2d gui_size;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_ROOT_H
