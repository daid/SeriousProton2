#include <sp2/graphics/gui/layout/verticalflow.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("verticalflow", VerticalFlowLayout);

void VerticalFlowLayout::update(P<Widget> container, Rect2d rect)
{
    float y = rect.position.y + rect.size.y;
    double x0 = rect.position.x;
    double x1 = rect.position.x;
    for(P<Widget> w : container->getChildren())
    {
        if (!w || !w->isVisible())
            continue;
        float height = w->layout.size.y + w->layout.margin.top + w->layout.margin.bottom;
        if (y - height < 0)
        {
            x0 = x1;
            y = rect.position.y + rect.size.y;
        }
        basicLayout(Rect2d(x0, y - height, rect.size.x - x0, height), *w);
        x1 = std::max(x1, x0 + w->getRenderSize().x + w->layout.margin.right);
        y -= height;
    }
}

};//namespace gui
};//namespace sp
