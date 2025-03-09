#include <sp2/graphics/gui/layout/vertical.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

void VerticalLayout::update(P<Widget> container, Rect2d rect)
{
    float total_height = 0.0f;
    float fill_height = 0.0f;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float h = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
        total_height += h;
        if (w->layout.fill_height)
            fill_height += h;
    }
    float remaining_height = rect.size.y - total_height;
    float y = rect.position.y + rect.size.y;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float h = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
        if (w->layout.fill_height && fill_height > 0.0f)
            h += remaining_height * w->layout.size.y / fill_height;
        basicLayout(Rect2d(rect.position.x, y - h, rect.size.x, h), *w);
        y = w->getPosition2D().y - w->layout.margin.bottom;
    }
}

}//namespace gui
}//namespace sp
