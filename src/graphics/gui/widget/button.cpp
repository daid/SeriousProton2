#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("button", Button);

Button::Button(P<Container> parent)
: Widget(parent)
{
    loadThemeData("button");
    text_size = -1;
}

void Button::setLabel(string label)
{
    this->label = label;
}

void Button::setAttribute(const string& key, const string& value)
{
    if (key == "label" || key == "caption")
    {
        label = value;
    }
    else if (key == "text_size" || key == "text.size")
    {
        text_size = stringutil::convert::toFloat(value);
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Button::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    renderStretched(window, layout.rect, t.background_image, t.background_color);
    renderText(window, layout.rect, Alignment::Center, label, t.font, text_size < 0 ? t.text_size : text_size, t.forground_color);
}

bool Button::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    return true;
}

void Button::onPointerUp(sf::Vector2f position, int id)
{
    if (layout.rect.contains(position) && isEnabled())
        runCallback(Variant());
}

};//!namespace gui
};//!namespace sp
