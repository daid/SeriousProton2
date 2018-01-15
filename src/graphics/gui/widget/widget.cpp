#include <sp2/graphics/gui/widget/widget.h>
#include <sp2/graphics/gui/graphicslayer.h>
#include <sp2/graphics/gui/layout/layout.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/assert.h>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Sleep.hpp>
#include <limits>

namespace sp {
namespace gui {

WidgetClassRegistry* WidgetClassRegistry::first;
float Widget::text_scale_factor = 0.5;

SP_REGISTER_WIDGET("", Widget);

Widget::Widget()
{
    layout.span = sf::Vector2i(1, 1);
    layout.margin_left = layout.margin_right = layout.margin_top = layout.margin_bottom = 0;
    layout.max_size.x = layout.max_size.y = std::numeric_limits<float>::max();
    layout.alignment = Alignment::TopLeft;
    layout.fill_width = layout.fill_height = layout.lock_aspect_ratio = layout.match_content_size = false;

    layout.anchor_point = Alignment::TopLeft;
    
    layout_manager = nullptr;
    visible = true;
    enabled = true;
    focus = false;
    focusable = false;
    hover = false;
    
    theme_name = "default";
}

Widget::Widget(P<Widget> parent)
: Widget()
{
    if (!parent)
    {
        sp2assert(GraphicsLayer::default_gui_layer, "Need to create a <sp::gui::GraphicsLayer> before Widgets can be created");
        parent = GraphicsLayer::default_gui_layer->root;
    }

    this->parent = parent;
    parent->children.add(this);
    
    theme_name = parent->theme_name;
}

Widget::~Widget()
{
    if (layout_manager)
        delete layout_manager;
}

void Widget::loadThemeData(string name)
{
    theme_data_name = name;
    theme = Theme::getTheme(theme_name)->getData(theme_data_name);
}

void Widget::setFocusable(bool value)
{
    focusable = value;
    if (!focusable)
        focus = false;
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
    if (hover)
        return State::Hovered;
    if (focus)
        return State::Focused;
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

void Widget::setVisible(bool visible)
{
    this->visible = visible;
}

void Widget::show()
{
    visible = true;
}

void Widget::hide()
{
    visible = false;
}

bool Widget::isVisible()
{
    return visible;
}

void Widget::setEnable(bool enable)
{
    enabled = enable;
}

void Widget::enable()
{
    enabled = true;
}

void Widget::disable()
{
    enabled = false;
}

bool Widget::isEnabled()
{
    return enabled;
}

void Widget::setID(string id)
{
    this->id = id;
}

void Widget::setEventCallback(Callback callback)
{
    this->callback = callback;
}

void Widget::setAttribute(const string& key, const string& value)
{
    if (key == "size")
    {
        layout.size = stringutil::convert::toVector2f(value);
    }
    else if (key == "position")
    {
        layout.position = stringutil::convert::toVector2f(value);
    }
    else if (key == "margin")
    {
        auto values = value.split(",", 3);
        if (values.size() == 1)
        {
            layout.margin_top = layout.margin_bottom = layout.margin_left = layout.margin_right = stringutil::convert::toFloat(values[0].strip());
        }
        else if (values.size() == 2)
        {
            layout.margin_left = layout.margin_right = stringutil::convert::toFloat(values[0].strip());
            layout.margin_top = layout.margin_bottom = stringutil::convert::toFloat(values[1].strip());
        }
        else if (values.size() == 3)
        {
            layout.margin_left = layout.margin_right = stringutil::convert::toFloat(values[0].strip());
            layout.margin_top = stringutil::convert::toFloat(values[1].strip());
            layout.margin_bottom = stringutil::convert::toFloat(values[2].strip());
        }
        else if (values.size() == 4)
        {
            layout.margin_left = stringutil::convert::toFloat(values[0].strip());
            layout.margin_right = stringutil::convert::toFloat(values[1].strip());
            layout.margin_top = stringutil::convert::toFloat(values[2].strip());
            layout.margin_bottom = stringutil::convert::toFloat(values[3].strip());
        }
    }
    else if (key == "span")
    {
        layout.span = stringutil::convert::toVector2i(value);
    }
    else if (key == "alignment")
    {
        if (value == "topleft") layout.alignment = Alignment::TopLeft;
        else if (value == "top") layout.alignment = Alignment::Top;
        else if (value == "right") layout.alignment = Alignment::TopRight;
        else if (value == "left") layout.alignment = Alignment::Left;
        else if (value == "center") layout.alignment = Alignment::Center;
        else if (value == "right") layout.alignment = Alignment::Right;
        else if (value == "bottomleft") layout.alignment = Alignment::BottomLeft;
        else if (value == "bottom") layout.alignment = Alignment::Bottom;
        else if (value == "bottomright") layout.alignment = Alignment::BottomRight;
    }
    else if (key == "layout")
    {
        LayoutClassRegistry* reg;
        for(reg = LayoutClassRegistry::first; reg != nullptr; reg = reg->next)
        {
            if (value == reg->name)
                break;
        }
        if (reg)
        {
            if (layout_manager)
                delete layout_manager;
            layout_manager = reg->creation_function();
        }else{
            LOG(Error, "Failed to find layout type:", value);
        }
    }
    else if (key == "match_content_size")
    {
        layout.match_content_size = stringutil::convert::toBool(value);
    }
    else if (key == "stretch")
    {
        layout.fill_height = layout.fill_width = stringutil::convert::toBool(value);
    }
    else if (key == "fill_height")
    {
        layout.fill_height = stringutil::convert::toBool(value);
    }
    else if (key == "fill_width")
    {
        layout.fill_width = stringutil::convert::toBool(value);
    }
    else if (key == "theme")
    {
        if (theme_name != value)
        {
            theme_name = value;
            theme = Theme::getTheme(theme_name)->getData(theme_data_name);
        }
    }
    else if (key == "theme_data")
    {
        if (theme_data_name != value)
        {
            theme_data_name = value;
            theme = Theme::getTheme(theme_name)->getData(theme_data_name);
        }
    }
    else
    {
        LOG(Warning, "Tried to set unknown attribute:", key, "to", value);
    }
}

P<Widget> Widget::getWidgetWithID(const string& id)
{
    if (this->id == id)
        return this;
    for(Widget* child : children)
    {
        P<Widget> w = child->getWidgetWithID(id);
        if (w)
            return w;
    }
    return nullptr;
}

#ifdef DEBUG
void Widget::setupAutoReload(P<Widget> widget, const string& resource_name, const string& root_id)
{
    AutoReloadData data;
    data.last_modify_time = io::ResourceProvider::getModifyTime(resource_name);
    data.resource_name = resource_name;
    data.root_id = root_id;
    data.widget = widget;
    auto_reload.push_back(data);
}
#endif

void Widget::updateLayout()
{
#ifdef DEBUG
    for(auto& data : auto_reload)
    {
        sf::Time modify_time = io::ResourceProvider::getModifyTime(data.resource_name);
        if (modify_time == data.last_modify_time)
            continue;
        LOG(Info, "Reloading:", data.resource_name, data.root_id);
        sf::sleep(sf::seconds(0.1));
        delete *data.widget;
        data.widget = sp::gui::Loader::load(data.resource_name, data.root_id, this);
        if (data.widget)
            data.last_modify_time = modify_time;
    }
#endif

    if (!layout_manager && !children.size())
        return;
    if (!layout_manager)
        layout_manager = new Layout();
    if (layout.match_content_size)
    {
        layout_manager->update(this, layout.rect);
        sf::FloatRect content_size(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
        for(Widget* child : children)
        {
            if (child->isVisible())
            {
                content_size.left = std::min(content_size.left, child->layout.rect.left - child->layout.margin_left);
                content_size.top = std::min(content_size.top, child->layout.rect.top - child->layout.margin_top);
                content_size.width = std::max(content_size.width + content_size.left, child->layout.rect.left + child->layout.rect.width + child->layout.margin_right) - content_size.left;
                content_size.height = std::max(content_size.height + content_size.top, child->layout.rect.top + child->layout.rect.height + child->layout.margin_bottom) - content_size.top;
            }
        }
        layout.rect = content_size;
        layout.size.x = content_size.width;
        layout.size.y = content_size.height;
    }else{
        layout_manager->update(this, layout.rect);
    }
    
    for(Widget* child : children)
    {
        if (child->isVisible())
            child->updateLayout();
    }
}

void Widget::runCallback(Variant v)
{
    if (callback)
        callback(v);
}

void Widget::renderStretched(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, Color color)
{
    if (texture == "")
        return;
    if (rect.width >= rect.height)
    {
        renderStretchedH(window, rect, texture, color);
    }else{
        renderStretchedV(window, rect, texture, color);
    }
}

void Widget::renderStretchedH(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, Color color)
{
    if (texture == "")
        return;
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

void Widget::renderStretchedV(sf::RenderTarget& window, const sf::FloatRect& rect, const string& texture, Color color)
{
    if (texture == "")
        return;
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

void Widget::renderStretchedHV(sf::RenderTarget& window, const sf::FloatRect& rect, float corner_size, const string& texture, Color color)
{
    if (texture == "")
        return;
    sf::Texture* texture_ptr = textureManager.get(texture);
    sf::Vector2f texture_size = sf::Vector2f(texture_ptr->getSize());
    sf::VertexArray a(sf::TrianglesStrip, 8);

    for(int n=0; n<8; n++)
        a[n].color = color;
    
    corner_size = std::min(corner_size, rect.height / 2.0f);
    corner_size = std::min(corner_size, rect.width / 2.0f);
    
    a[0].position = sf::Vector2f(rect.left, rect.top);
    a[1].position = sf::Vector2f(rect.left, rect.top + corner_size);
    a[2].position = sf::Vector2f(rect.left + corner_size, rect.top);
    a[3].position = sf::Vector2f(rect.left + corner_size, rect.top + corner_size);
    a[4].position = sf::Vector2f(rect.left + rect.width - corner_size, rect.top);
    a[5].position = sf::Vector2f(rect.left + rect.width - corner_size, rect.top + corner_size);
    a[6].position = sf::Vector2f(rect.left + rect.width, rect.top);
    a[7].position = sf::Vector2f(rect.left + rect.width, rect.top + corner_size);
    
    a[0].texCoords = sf::Vector2f(0, 0);
    a[1].texCoords = sf::Vector2f(0, texture_size.y / 2.0);
    a[2].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[3].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y / 2.0);
    a[4].texCoords = sf::Vector2f(texture_size.x / 2, 0);
    a[5].texCoords = sf::Vector2f(texture_size.x / 2, texture_size.y / 2.0);
    a[6].texCoords = sf::Vector2f(texture_size.x, 0);
    a[7].texCoords = sf::Vector2f(texture_size.x, texture_size.y / 2.0);

    window.draw(a, texture_ptr);

    a[0].position.y = rect.top + rect.height - corner_size;
    a[2].position.y = rect.top + rect.height - corner_size;
    a[4].position.y = rect.top + rect.height - corner_size;
    a[6].position.y = rect.top + rect.height - corner_size;
    
    a[0].texCoords.y = texture_size.y / 2.0;
    a[2].texCoords.y = texture_size.y / 2.0;
    a[4].texCoords.y = texture_size.y / 2.0;
    a[6].texCoords.y = texture_size.y / 2.0;
    
    window.draw(a, texture_ptr);

    a[1].position.y = rect.top + rect.height;
    a[3].position.y = rect.top + rect.height;
    a[5].position.y = rect.top + rect.height;
    a[7].position.y = rect.top + rect.height;
    
    a[1].texCoords.y = texture_size.y;
    a[3].texCoords.y = texture_size.y;
    a[5].texCoords.y = texture_size.y;
    a[7].texCoords.y = texture_size.y;
    
    window.draw(a, texture_ptr);
}

void Widget::renderText(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, Color color)
{
    sf::Text text_element(text, *fontManager.get(font_name), text_size / text_scale_factor);
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
        y = rect.top + rect.height / 2.0 - text_size + text_size * 0.3;
        break;
    }
    
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        x = rect.left - text_element.getLocalBounds().left * text_scale_factor;
        break;
    case Alignment::TopRight:
    case Alignment::BottomRight:
    case Alignment::Right:
        x = rect.left + rect.width - text_element.getLocalBounds().width * text_scale_factor - text_element.getLocalBounds().left * text_scale_factor;
        break;
    case Alignment::Top:
    case Alignment::Bottom:
    case Alignment::Center:
        x = rect.left + rect.width / 2.0 - text_element.getLocalBounds().width * text_scale_factor / 2.0 - text_element.getLocalBounds().left * text_scale_factor;
        break;
    }
    text_element.setScale(text_scale_factor, text_scale_factor);
    text_element.setPosition(x, y);
    text_element.setFillColor(color);
    window.draw(text_element);
}

void Widget::renderTextVertical(sf::RenderTarget& window, const sf::FloatRect& rect, Alignment alignment, const string& text, const string& font_name, float text_size, Color color)
{
    sf::Text text_element(text, *fontManager.get(font_name), text_size / text_scale_factor);
    text_element.setRotation(-90);
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
        y = rect.top + text_element.getLocalBounds().width * text_scale_factor + text_element.getLocalBounds().left * text_scale_factor;
        break;
    case Alignment::BottomLeft:
    case Alignment::BottomRight:
    case Alignment::Bottom:
        y = rect.top + rect.height + text_element.getLocalBounds().left * text_scale_factor;
        break;
    case Alignment::Left:
    case Alignment::Right:
    case Alignment::Center:
        y = rect.top + rect.height / 2.0 + text_element.getLocalBounds().width * text_scale_factor / 2.0 + text_element.getLocalBounds().left * text_scale_factor;
        break;
    }
    
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        x = rect.left + text_size * 0.3;
        break;
    case Alignment::TopRight:
    case Alignment::BottomRight:
    case Alignment::Right:
        x = rect.left + rect.width - text_size;
        break;
    case Alignment::Top:
    case Alignment::Bottom:
    case Alignment::Center:
        x = rect.left + rect.width / 2.0 - text_size * 0.7;
        break;
    }
    text_element.setScale(text_scale_factor, text_scale_factor);
    text_element.setPosition(x, y);
    text_element.setFillColor(color);
    window.draw(text_element);
}

};//!namespace gui
};//!namespace sp
