#include <sp2/graphics/gui/layout/horizontal.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("horizontal", HorizontalLayout);

void HorizontalLayout::update(P<Container> container, const sf::FloatRect& rect)
{
    float x = rect.left;
    for(Widget* w : container->children)
    {
        if (!w->isVisible())
            continue;
        float width = w->layout.size.x + w->layout.margin_left + w->layout.margin_right;
        sf::FloatRect r(x, rect.top, width, rect.height);
        basicLayout(r, w);
        x += width;
    }
}

};//!namespace gui
};//!namespace sp
