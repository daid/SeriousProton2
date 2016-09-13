#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

Button::Button(P<Container> parent)
: Widget(parent)
{
    loadThemeData("button");
}

void Button::setLabel(string label)
{
    this->label = label;
}

void Button::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    renderStretched(window, layout.rect, t.background_image, t.background_color);
    renderText(window, layout.rect, Alignment::Center, label, t.font, t.text_size, t.forground_color);
}

bool Button::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    return true;
}

void Button::onPointerUp(sf::Vector2f position, int id)
{
}

};//!namespace gui
};//!namespace sp
