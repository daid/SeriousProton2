#include <sp2/graphics/gui/widget/button.h>

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
    
}

};//!namespace gui
};//!namespace sp
