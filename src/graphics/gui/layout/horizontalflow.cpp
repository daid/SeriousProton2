#include <sp2/graphics/gui/layout/horizontalflow.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

void HorizontalFlowLayout::update(P<Widget> container, Rect2d rect)
{
    double x = rect.position.x;
    double y0 = rect.position.y + rect.size.y;
    double y1 = rect.position.y + rect.size.y;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        double width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
        if (x + width > rect.size.x)
        {
            y0 = y1;
            x = rect.position.x;
        }
        double height = y0 - rect.position.y;
        basicLayout(Rect2d(x, y0 - height, width, height), *w);
        y1 = std::min(y1, y0 - w->getRenderSize().y - w->layout.margin.bottom);
        x += width;
    }
}

}//namespace gui
}//namespace sp
