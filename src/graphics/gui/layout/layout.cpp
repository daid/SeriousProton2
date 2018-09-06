#include <sp2/graphics/gui/layout/layout.h>

namespace sp {
namespace gui {

LayoutClassRegistry* LayoutClassRegistry::first;

SP_REGISTER_LAYOUT("default", Layout);

void Layout::update(P<Widget> container, Rect2d rect)
{
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        basicLayout(rect, *w);
    }
}

void Layout::basicLayout(Rect2d rect, Widget* widget)
{
    Vector2d result_position;
    Vector2d result_size;
    switch(widget->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        result_position.x = rect.position.x + widget->layout.position.x + widget->layout.margin.left;
        if (widget->layout.fill_width)
            result_size.x = rect.size.x - widget->layout.margin.left - widget->layout.margin.right + widget->layout.position.x;
        else
            result_size.x = widget->layout.size.x;
        break;
    case Alignment::Top:
    case Alignment::Center:
    case Alignment::Bottom:
        if (widget->layout.fill_width)
            result_size.x = rect.size.x - widget->layout.margin.left - widget->layout.margin.right;
        else
            result_size.x = widget->layout.size.x;
        result_position.x = rect.position.x + rect.size.x / 2.0f - result_size.x / 2.0f;
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        result_position.x = rect.position.x + widget->layout.position.x + widget->layout.margin.left;
        if (widget->layout.fill_width)
            result_size.x = rect.size.x - widget->layout.margin.left - widget->layout.margin.right + widget->layout.position.x;
        else
            result_size.x = widget->layout.size.x;
        result_position.x = rect.position.x + rect.size.x - widget->layout.margin.right + widget->layout.position.x - result_size.x;
        break;
    }

    switch(widget->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        result_position.y = rect.position.y - widget->layout.position.y + widget->layout.margin.top;
        if (widget->layout.fill_height)
            result_size.y = rect.size.y - widget->layout.margin.top - widget->layout.margin.bottom - widget->layout.position.y;
        else
            result_size.y = widget->layout.size.y;
        result_position.y = rect.position.y + rect.size.y - widget->layout.margin.top - widget->layout.position.y - result_size.y;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        if (widget->layout.fill_height)
            result_size.y = rect.size.y - widget->layout.margin.top - widget->layout.margin.bottom;
        else
            result_size.y = widget->layout.size.y;
        result_position.y = rect.position.y + rect.size.y / 2.0f - result_size.y / 2.0f;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        result_position.y = rect.position.y - widget->layout.position.y + widget->layout.margin.bottom;
        if (widget->layout.fill_height)
            result_size.y = rect.size.y - widget->layout.margin.top - widget->layout.margin.bottom - widget->layout.position.y;
        else
            result_size.y = widget->layout.size.y;
        break;
    }
    if (widget->layout.lock_aspect_ratio)
    {
        double aspect = widget->layout.size.x / widget->layout.size.y;
        if (widget->layout.fill_height && widget->layout.fill_width)
        {
            double current_aspect = result_size.x / result_size.y;
            if (current_aspect > aspect)
                result_size.x = result_size.y / aspect;
            else
                result_size.y = result_size.x * aspect;
        }
        else if (widget->layout.fill_height)
        {
            result_size.x = result_size.y / aspect;
        }
        else if (widget->layout.fill_width)
        {
            result_size.y = result_size.x * aspect;
        }
    }
    widget->updateLayout(result_position, result_size);
}

};//namespace gui
};//namespace sp
