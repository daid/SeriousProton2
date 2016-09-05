#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/graphicslayer.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>

#include <limits>

namespace sp {
namespace gui {

Widget::Widget()
{
    layout.margin_left = layout.margin_right = layout.margin_top = layout.margin_bottom = 0;
    layout.max_size.x = layout.max_size.y = std::numeric_limits<float>::max();
    layout.alignment = Alignment::TopLeft;
    layout.fill_width = layout.fill_height = false;

    layout.anchor_point = Alignment::TopLeft;
    
    layout_manager = nullptr;
    enabled = true;
    focus = false;
    hover = false;
}

Widget::Widget(P<Widget> parent)
: Widget()
{
    if (!parent)
    {
        parent = GraphicsLayer::default_gui_layer->root;
    }

    this->parent = parent;
    parent->children.add(this);
    
    theme_name = parent->theme_name;
}

void Widget::loadThemeData(string name)
{
    theme_data_name = name;
    theme = Theme::getTheme(theme_name)->getData(theme_data_name);
}

void Widget::updateLayout()
{
    if (!layout_manager && !children.size())
        return;
    if (!layout_manager)
        layout_manager = new Layout();
    layout_manager->update(this, sf::FloatRect(layout.position.x, layout.position.y, layout.size.x, layout.size.y));
    
    for(Widget* child : children)
    {
        child->updateLayout();
    }
}

void Widget::render(sf::RenderTarget& window)
{
}

};//!namespace gui
};//!namespace sp
