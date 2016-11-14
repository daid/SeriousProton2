#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

LayoutClassRegistry* LayoutClassRegistry::first;

SP_REGISTER_LAYOUT("default", Layout);

void Layout::update(P<Container> container, const sf::FloatRect& rect)
{
    for(Widget* w : container->children)
    {
        basicLayout(rect, w);
    }
}

void Layout::basicLayout(const sf::FloatRect& rect, Widget* widget)
{
    switch(widget->layout.alignment)
    {
    case Widget::Alignment::TopLeft:
    case Widget::Alignment::BottomLeft:
    case Widget::Alignment::Left:
        widget->layout.rect.left = rect.left + widget->layout.position.x + widget->layout.margin_left;
        if (widget->layout.fill_width)
            widget->layout.rect.width = rect.width - widget->layout.margin_left - widget->layout.margin_right + widget->layout.position.x;
        else
            widget->layout.rect.width = widget->layout.size.x;
        break;
    case Widget::Alignment::Top:
    case Widget::Alignment::Center:
    case Widget::Alignment::Bottom:
        if (widget->layout.fill_width)
            widget->layout.rect.width = rect.width - widget->layout.margin_left - widget->layout.margin_right;
        else
            widget->layout.rect.width = widget->layout.size.x;
        widget->layout.rect.left = rect.left + rect.width / 2.0f - widget->layout.rect.width / 2.0f;
        break;
    case Widget::Alignment::TopRight:
    case Widget::Alignment::Right:
    case Widget::Alignment::BottomRight:
        widget->layout.rect.left = rect.left + widget->layout.position.x + widget->layout.margin_left;
        if (widget->layout.fill_width)
            widget->layout.rect.width = rect.width - widget->layout.margin_left - widget->layout.margin_right + widget->layout.position.x;
        else
            widget->layout.rect.width = widget->layout.size.x;
        widget->layout.rect.left = rect.left + rect.width - widget->layout.margin_right + widget->layout.position.x - widget->layout.rect.width;
        break;
    }

    switch(widget->layout.alignment)
    {
    case Widget::Alignment::TopLeft:
    case Widget::Alignment::Top:
    case Widget::Alignment::TopRight:
        widget->layout.rect.top = rect.top + widget->layout.position.y + widget->layout.margin_top;
        if (widget->layout.fill_height)
            widget->layout.rect.height = rect.height - widget->layout.margin_top - widget->layout.margin_bottom + widget->layout.position.y;
        else
            widget->layout.rect.height = widget->layout.size.y;
        break;
    case Widget::Alignment::Left:
    case Widget::Alignment::Center:
    case Widget::Alignment::Right:
        if (widget->layout.fill_height)
            widget->layout.rect.height = rect.height - widget->layout.margin_top - widget->layout.margin_bottom;
        else
            widget->layout.rect.height = widget->layout.size.y;
        widget->layout.rect.top = rect.top + rect.height / 2.0f - widget->layout.rect.height / 2.0f;
        break;
    case Widget::Alignment::BottomLeft:
    case Widget::Alignment::Bottom:
    case Widget::Alignment::BottomRight:
        widget->layout.rect.top = rect.top + widget->layout.position.y + widget->layout.margin_top;
        if (widget->layout.fill_height)
            widget->layout.rect.height = rect.height - widget->layout.margin_top - widget->layout.margin_bottom + widget->layout.position.y;
        else
            widget->layout.rect.height = widget->layout.size.y;
        widget->layout.rect.top = rect.top + rect.height - widget->layout.margin_bottom + widget->layout.position.y - widget->layout.rect.height;
        break;
    }
    if (widget->layout.lock_aspect_ratio)
    {
        
    }
}

};//!namespace gui
};//!namespace sp
