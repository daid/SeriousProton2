#include <sp2/graphics/gui/layout/verticalflow.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("verticalflow", VerticalFlowLayout);

void VerticalFlowLayout::update(P<Widget> container, Vector2d size)
{
    float y = size.y;
    double x0 = 0;
    double x1 = 0;
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        float height = w->layout.size.y + w->layout.margin_top + w->layout.margin_bottom;
        if (y - height < 0)
        {
            x0 = x1;
            y = size.y;
        }
        basicLayout(Vector2d(x0, y - height), Vector2d(size.x - x0, height), *w);
        x1 = std::max(x1, x0 + w->getRenderSize().x + w->layout.margin_right);
        y -= height;
    }
}

};//namespace gui
};//namespace sp
