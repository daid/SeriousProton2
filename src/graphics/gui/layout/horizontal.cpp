#include <sp2/graphics/gui/layout/horizontal.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

void HorizontalLayout::update(P<Widget> container, Rect2d rect)
{
    float total_width = 0.0f;
    float fill_width = 0.0f;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
        total_width += width;
        if (w->layout.fill_width)
            fill_width += w->layout.size.x;
    }
    float remaining_width = rect.size.x - total_width;
    float x = rect.position.x;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
        if (w->layout.fill_width && fill_width > 0.0f)
            width += remaining_width * w->layout.size.x / fill_width;
        basicLayout(Rect2d(x, rect.position.y, width, rect.size.y), *w);
        x = w->getPosition2D().x + w->getRenderSize().x + w->layout.margin.right;
    }
}

}//namespace gui
}//namespace sp
