#include <sp2/graphics/gui/layout/horizontal.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("horizontal", HorizontalLayout);

void HorizontalLayout::update(P<Widget> container, Vector2d size)
{
    float x = 0;
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        float width = w->layout.size.x + w->layout.margin_left + w->layout.margin_right;
        basicLayout(Vector2d(x, 0), Vector2d(width, size.y), *w);
        x += width;
    }
}

};//!namespace gui
};//!namespace sp
