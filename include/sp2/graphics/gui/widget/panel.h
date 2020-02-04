#ifndef SP2_GRAPHICS_GUI_PANEL_H
#define SP2_GRAPHICS_GUI_PANEL_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class Panel : public Widget
{
public:
    Panel(P<Widget> parent);
    
    virtual void updateRenderData() override;
    virtual bool onPointerMove(Vector2d position, int id) override;
    virtual bool onPointerDown(io::Pointer::Button button, Vector2d position, int id) override;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_PANEL_H

