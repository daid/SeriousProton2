#ifndef SP2_GRAPHICS_GUI_LAYOUT_HORIZONTAL_H
#define SP2_GRAPHICS_GUI_LAYOUT_HORIZONTAL_H

#include <sp2/graphics/gui/layout/layout.h>

namespace sp {
namespace gui {

class HorizontalLayout : public Layout
{
public:
    virtual void update(P<Widget> container, Rect2d rect) override;
};

}//namespace gui
}//namespace sp

#endif//SP2_GRAPHICS_GUI_LAYOUT_HORIZONTAL_H
