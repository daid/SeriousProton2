#include <sp2/graphics/gui/layout/vertical.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("vertical", VerticalLayout);

void VerticalLayout::update(P<Widget> container, Vector2d size)
{
    float y = size.y;
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        float h = w->layout.size.y + w->layout.margin_top + w->layout.margin_bottom;
        basicLayout(Vector2d(0, y - h), Vector2d(size.x, h), *w);
        y -= h;
    }
}

};//namespace gui
};//namespace sp
