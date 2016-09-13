#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/graphicslayer.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Text.hpp>
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
    sf::Texture* texture_ptr = textureManager.get(texture);
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
    sf::Texture* texture_ptr = textureManager.get(texture);
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

void Widget::renderText(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, sf::Color color)
{
    sf::Text textElement(text, *fontManager.get(font_name), text_size);
    float y = 0;
    float x = 0;
    
    //The "base line" of the text draw is the "Y position where the text is drawn" + font_size.
    //The height of normal text is 70% of the font_size.
    //So use those properties to align the text. Depending on the localbounds does not work.
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::TopRight:
    case Alignment::Top:
        y = rect.top - 0.3 * text_size;
        break;
    case Alignment::BottomLeft:
    case Alignment::BottomRight:
    case Alignment::Bottom:
        y = rect.top + rect.height - text_size;
        break;
    case Alignment::Left:
    case Alignment::Right:
    case Alignment::Center:
        y = rect.top + rect.height / 2.0 - text_size + text_size * 0.35;
        break;
    }
    
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        x = rect.left - textElement.getLocalBounds().left;
        break;
    case Alignment::TopRight:
    case Alignment::BottomRight:
    case Alignment::Right:
        x = rect.left + rect.width - textElement.getLocalBounds().width - textElement.getLocalBounds().left;
        break;
    case Alignment::Top:
    case Alignment::Bottom:
    case Alignment::Center:
        x = rect.left + rect.width / 2.0 - textElement.getLocalBounds().width / 2.0 - textElement.getLocalBounds().left;
        break;
    }
    textElement.setPosition(x, y);
    textElement.setFillColor(color);
    window.draw(textElement);
}

};//!namespace gui
};//!namespace sp
