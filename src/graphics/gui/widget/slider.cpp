#include <sp2/graphics/gui/widget/slider.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("slider", Slider);

Slider::Slider(P<Container> parent)
: Widget(parent)
{
    loadThemeData("slider");

    min_value = 0.0;
    max_value = 1.0;
    value = 0.5;
}

void Slider::setAttribute(const string& key, const string& value)
{
    if (key == "min" || key == "min_value")
    {
        min_value = value.toFloat();
    }
    else if (key == "max" || key == "max_value")
    {
        max_value = value.toFloat();
    }
    else if (key == "value")
    {
        this->value = value.toFloat();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Slider::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    renderStretched(window, layout.rect, t.background_image, t.background_color);
    sf::FloatRect rect(layout.rect.left, layout.rect.top, std::min(layout.rect.width, layout.rect.height), std::min(layout.rect.width, layout.rect.height));
    
    float f = (value - min_value) / (max_value - min_value);
    if (layout.rect.width > layout.rect.height)
    {
        rect.left += (layout.rect.width - layout.rect.height) * f;
    }
    else
    {
        rect.top += (layout.rect.height - layout.rect.width) * f;
    }
    
    renderStretched(window, rect, t.forground_image, t.forground_color);
}

bool Slider::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    onPointerDrag(position, id);
    return true;
}

void Slider::onPointerDrag(sf::Vector2f position, int id)
{
    float f;
    if (layout.rect.width > layout.rect.height)
        f = (position.x - layout.rect.left - layout.rect.height / 2.0) / (layout.rect.width - layout.rect.height);
    else
        f = (position.y - layout.rect.top - layout.rect.width / 2.0) / (layout.rect.height - layout.rect.width);
    f = std::max(0.0f, std::min(f, 1.0f));
    value = (f * (max_value - min_value)) + min_value;
    
    runCallback(value);
}

void Slider::onPointerUp(sf::Vector2f position, int id)
{
}

};//!namespace gui
};//!namespace sp
