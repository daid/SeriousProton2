#ifndef SP2_GRAPHICS_GUI_LAYOUT_GRID_H
#define SP2_GRAPHICS_GUI_LAYOUT_GRID_H

#include <sp2/graphics/gui/layout/layout.h>

namespace sp {
namespace gui {

class GridLayout : public Layout
{
public:
    virtual void update(P<Container> container, const sf::FloatRect& rect) override;
};

};//!namespace gui
};//!namespace sp

#endif//SP2_GRAPHICS_GUI_LAYOUT_GRID_H
