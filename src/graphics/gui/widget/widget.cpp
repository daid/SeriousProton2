#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/graphicslayer.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
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

void Widget::render(sf::RenderTarget& window)
{
}

bool Widget::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    return false;
}

void Widget::onPointerDrag(sf::Vector2f position, int id)
{
}

void Widget::onPointerUp(sf::Vector2f position, int id)
{
}

Widget::State Widget::getState() const
{
    if (!enabled)
        return State::Disabled;
    if (focus)
        return State::Focused;
    if (hover)
        return State::Hovered;
    return State::Normal;
}

void Widget::setPosition(float x, float y, Alignment alignment)
{
    layout.position.x = x;
    layout.position.y = y;
    layout.alignment = alignment;
}

void Widget::setPosition(sf::Vector2f v, Alignment alignment)
{
    layout.position = v;
    layout.alignment = alignment;
}

void Widget::setSize(float x, float y)
{
    layout.size.x = x;
    layout.size.y = y;
}

void Widget::setSize(sf::Vector2f v)
{
    layout.size = v;
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

void Widget::renderStretched(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color)
{
    if (rect.width >= rect.height)
    {
        renderStretchedH(window, rect, texture, color);
    }else{
        renderStretchedV(window, rect, texture, color);
    }
}

void Widget::renderStretchedH(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color)
{
    sf::Texture* texture_ptr = TextureManager::get(texture);
    sf::Vector2f texture_size = sf::Vector2f(texture_ptr->getSize());
    sf::VertexArray a(sf::TrianglesStrip, 8);
    
    float w = rect.height / 2.0f;
    if (w * 2 > rect.width)
        w = rect.width / 2.0f;
    a[0].position = sf::Vector2f(rect.left, rect.top);
    a[1].position = sf::Vector2f(rect.left, rect.top + rect.height);
    a[2].position = sf::Vector2f(rect.left + w, rect.top);
    a[3].position = sf::Vector2f(rect.left + w, rect.top + rect.height);
    a[4].position = sf::Vector2f(rect.left + rect.width - w, rect.top);
    a[5].position = sf::Vector2f(rect.left + rect.width - w, rect.top + rect.height);
    a[6].position = sf::Vector2f(rect.left + rect.width, rect.top);
    a[7].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);
    
    a[0].texCoords = sf::Vector2f(0, 0);
    a[1].texCoords = sf::Vector2f(0, texture_size.y);
    a[2].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[3].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);
    a[4].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[5].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);
    a[6].texCoords = sf::Vector2f(texture_size.x, 0);
    a[7].texCoords = sf::Vector2f(texture_size.x, texture_size.y);

    for(int n=0; n<8; n++)
        a[n].color = color;
    
    window.draw(a, texture_ptr);
}

void Widget::renderStretchedV(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, sf::Color color)
{
    sf::Texture* texture_ptr = TextureManager::get(texture);
    sf::Vector2f texture_size = sf::Vector2f(texture_ptr->getSize());
    sf::VertexArray a(sf::TrianglesStrip, 8);
    
    float h = rect.width / 2.0;
    if (h * 2 > rect.height)
        h = rect.height / 2.0f;
    a[0].position = sf::Vector2f(rect.left, rect.top);
    a[1].position = sf::Vector2f(rect.left + rect.width, rect.top);
    a[2].position = sf::Vector2f(rect.left, rect.top + h);
    a[3].position = sf::Vector2f(rect.left + rect.width, rect.top + h);
    a[4].position = sf::Vector2f(rect.left, rect.top + rect.height - h);
    a[5].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height - h);
    a[6].position = sf::Vector2f(rect.left, rect.top + rect.height);
    a[7].position = sf::Vector2f(rect.left + rect.width, rect.top + rect.height);

    a[0].texCoords = sf::Vector2f(0, 0);
    a[1].texCoords = sf::Vector2f(0, texture_size.y);
    a[2].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[3].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);
    a[4].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[5].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y);
    a[6].texCoords = sf::Vector2f(texture_size.x, 0);
    a[7].texCoords = sf::Vector2f(texture_size.x, texture_size.y);

    for(int n=0; n<8; n++)
        a[n].color = color;
    
    window.draw(a, texture_ptr);
}

};//!namespace gui
};//!namespace sp
