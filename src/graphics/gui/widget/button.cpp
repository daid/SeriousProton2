#include <sp2/graphics/gui/widget/button.h>
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
    sf::RectangleShape tmp(sf::Vector2f(layout.rect.width, layout.rect.height));
    tmp.setPosition(layout.rect.left, layout.rect.top);
    window.draw(tmp);
}

};//!namespace gui
};//!namespace sp
