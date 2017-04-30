#include <sp2/graphics/gui/layout/verticalflow.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("verticalflow", VerticalFlowLayout);

void VerticalFlowLayout::update(P<Container> container, const sf::FloatRect& rect)
{
    float y = rect.top;
    float x0 = rect.left;
    float x1 = rect.left;
    for(Widget* w : container->children)
    {
        if (!w->isVisible())
            continue;
        float height = w->layout.size.y + w->layout.margin_top + w->layout.margin_bottom;
        if (y + height > rect.top + rect.height)
        {
            x0 = x1;
            y = rect.top;
        }
        sf::FloatRect r(x0, y, rect.width, height);
        basicLayout(r, w);
        x1 = std::max(x1, w->layout.rect.left + w->layout.rect.width + w->layout.margin_right);
        y += height;
    }
}

};//!namespace gui
};//!namespace sp
