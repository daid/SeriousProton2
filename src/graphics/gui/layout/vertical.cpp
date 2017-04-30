#include <sp2/graphics/gui/layout/vertical.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("vertical", VerticalLayout);

void VerticalLayout::update(P<Container> container, const sf::FloatRect& rect)
{
    float y = rect.top;
    for(Widget* w : container->children)
    {
        if (!w->isVisible())
            continue;
        float h = w->layout.size.y + w->layout.margin_top + w->layout.margin_bottom;
        sf::FloatRect r(rect.left, y, rect.width, h);
        basicLayout(r, w);
        y += h;
    }
}

};//!namespace gui
};//!namespace sp
