#include <sp2/graphics/gui/widget/root.h>

namespace sp {
namespace gui {

RootWidget::RootWidget(P<Node> parent, Vector2d size)
: Widget(parent)
{
    gui_size = size;
    layout.match_content_size = false;
}

void RootWidget::onUpdate(float delta)
{
    layout.size = gui_size;
    updateLayout(Vector2d(0, 0), gui_size);
}

}//namespace gui
}//namespace sp
