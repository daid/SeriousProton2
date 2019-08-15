#ifndef SP2_GRAPHICS_GUI_ROOT_H
#define SP2_GRAPHICS_GUI_ROOT_H

#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

class RootWidget : public Widget
{
private:
    RootWidget(P<Node> parent, Vector2d size);
    
    virtual void onUpdate(float delta) override;

    Vector2d gui_size;

    friend class Scene;
};

};//namespace gui
};//namespace sp

#endif//SP2_GRAPHICS_GUI_ROOT_H
