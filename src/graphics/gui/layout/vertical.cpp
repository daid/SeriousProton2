#include <sp2/graphics/gui/layout/vertical.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("vertical", VerticalLayout);

void VerticalLayout::update(P<Widget> container, Rect2d rect)
{
    float y = rect.position.y + rect.size.y;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float h = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
        basicLayout(Rect2d(rect.position.x, y - h, rect.size.x, h), *w);
        y -= h;
    }
}

}//namespace gui
}//namespace sp
