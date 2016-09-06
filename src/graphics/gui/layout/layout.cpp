#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/widget/widget.h>

namespace sp {
namespace gui {

void Layout::update(P<Container> container, const sf::FloatRect& rect)
{
    for(Widget* w : container->children)
    {
        switch(w->layout.alignment)
        {
        case Widget::Alignment::TopLeft:
        case Widget::Alignment::BottomLeft:
        case Widget::Alignment::Left:
            w->layout.rect.left = rect.left + w->layout.position.x + w->layout.margin_left;
            if (w->layout.fill_width)
                w->layout.rect.width = rect.width - w->layout.rect.left - w->layout.margin_right;
            else
                w->layout.rect.width = w->layout.size.x;
            break;
        case Widget::Alignment::Top:
        case Widget::Alignment::Center:
        case Widget::Alignment::Bottom:
            if (w->layout.fill_width)
                w->layout.rect.width = rect.width - w->layout.margin_left - w->layout.margin_right;
            else
                w->layout.rect.width = w->layout.size.x;
            w->layout.rect.left = rect.width / 2.0f - w->layout.rect.width / 2.0f;
            break;
        case Widget::Alignment::TopRight:
        case Widget::Alignment::Right:
        case Widget::Alignment::BottomRight:
            w->layout.rect.left = rect.left + w->layout.position.x + w->layout.margin_left;
            if (w->layout.fill_width)
                w->layout.rect.width = rect.width - w->layout.margin_left - w->layout.margin_right + w->layout.position.x;
            else
                w->layout.rect.width = w->layout.size.x;
            w->layout.rect.left = rect.left + rect.width - w->layout.margin_right + w->layout.position.x - w->layout.rect.width;
            break;
        }

        switch(w->layout.alignment)
        {
        case Widget::Alignment::TopLeft:
        case Widget::Alignment::Top:
        case Widget::Alignment::TopRight:
            w->layout.rect.top = rect.top + w->layout.position.y + w->layout.margin_top;
            if (w->layout.fill_height)
                w->layout.rect.height = rect.height - w->layout.rect.top - w->layout.margin_bottom;
            else
                w->layout.rect.height = w->layout.size.y;
            break;
        case Widget::Alignment::Left:
        case Widget::Alignment::Center:
        case Widget::Alignment::Right:
            if (w->layout.fill_height)
                w->layout.rect.height = rect.height - w->layout.margin_top - w->layout.margin_bottom;
            else
                w->layout.rect.height = w->layout.size.y;
            w->layout.rect.top = rect.height / 2.0f - w->layout.rect.height / 2.0f;
            break;
        case Widget::Alignment::BottomLeft:
        case Widget::Alignment::Bottom:
        case Widget::Alignment::BottomRight:
            w->layout.rect.top = rect.top + w->layout.position.y + w->layout.margin_top;
            if (w->layout.fill_height)
                w->layout.rect.height = rect.height - w->layout.margin_top - w->layout.margin_bottom + w->layout.position.y;
            else
                w->layout.rect.height = w->layout.size.y;
            w->layout.rect.top = rect.top + rect.height - w->layout.margin_bottom + w->layout.position.y - w->layout.rect.height;
            break;
        }
    }
}

};//!namespace gui
};//!namespace sp
