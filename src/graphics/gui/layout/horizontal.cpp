#include <sp2/graphics/gui/layout/horizontal.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

SP_REGISTER_LAYOUT("horizontal", HorizontalLayout);

void HorizontalLayout::update(P<Widget> container, Rect2d rect)
{
    float x = rect.position.x;
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        float width = w->layout.size.x + w->layout.margin.left + w->layout.margin.right;
        basicLayout(Rect2d(x, rect.position.y, width, rect.size.y), *w);
        x += width;
    }
}

};//namespace gui
};//namespace sp
