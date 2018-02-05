#include <sp2/graphics/gui/layout/layout.h>

namespace sp {
namespace gui {

LayoutClassRegistry* LayoutClassRegistry::first;

SP_REGISTER_LAYOUT("default", Layout);

void Layout::update(P<Widget> container, Vector2d size)
{
    for(Node* n : container->getChildren())
    {
        P<Widget> w = P<Node>(n);
        if (!w || !w->isVisible())
            continue;
        basicLayout(Vector2d(0, 0), size, *w);
    }
}

void Layout::basicLayout(Vector2d position, Vector2d size, Widget* widget)
{
    Vector2d result_position;
    Vector2d result_size;
    switch(widget->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        result_position.x = position.x + widget->layout.position.x + widget->layout.margin_left;
        if (widget->layout.fill_width)
            result_size.x = size.x - widget->layout.margin_left - widget->layout.margin_right + widget->layout.position.x;
        else
            result_size.x = widget->layout.size.x;
        break;
    case Alignment::Top:
    case Alignment::Center:
    case Alignment::Bottom:
        if (widget->layout.fill_width)
            result_size.x = size.x - widget->layout.margin_left - widget->layout.margin_right;
        else
            result_size.x = widget->layout.size.x;
        result_position.x = position.x + size.x / 2.0f - result_size.x / 2.0f;
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        result_position.x = position.x + widget->layout.position.x + widget->layout.margin_left;
        if (widget->layout.fill_width)
            result_size.x = size.x - widget->layout.margin_left - widget->layout.margin_right + widget->layout.position.x;
        else
            result_size.x = widget->layout.size.x;
        result_position.x = position.x + size.x - widget->layout.margin_right + widget->layout.position.x - result_size.x;
        break;
    }

    switch(widget->layout.alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        result_position.y = position.y - widget->layout.position.y + widget->layout.margin_top;
        if (widget->layout.fill_height)
            result_size.y = size.y - widget->layout.margin_top - widget->layout.margin_bottom - widget->layout.position.y;
        else
            result_size.y = widget->layout.size.y;
        result_position.y = position.y + size.y - widget->layout.margin_top - widget->layout.position.y - result_size.y;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        if (widget->layout.fill_height)
            result_size.y = size.y - widget->layout.margin_top - widget->layout.margin_bottom;
        else
            result_size.y = widget->layout.size.y;
        result_position.y = position.y + size.y / 2.0f - result_size.y / 2.0f;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        result_position.y = position.y - widget->layout.position.y + widget->layout.margin_bottom;
        if (widget->layout.fill_height)
            result_size.y = size.y - widget->layout.margin_top - widget->layout.margin_bottom - widget->layout.position.y;
        else
            result_size.y = widget->layout.size.y;
        break;
    }
    if (widget->layout.lock_aspect_ratio)
    {
        if (widget->layout.fill_height)
            result_size.x = result_size.y / widget->layout.size.y * widget->layout.size.x;
        if (widget->layout.fill_width)
            result_size.y = result_size.y / widget->layout.size.x * widget->layout.size.y;
    }
    widget->updateLayout(result_position, result_size);
}

};//!namespace gui
};//!namespace sp
