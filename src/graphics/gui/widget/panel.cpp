#include <sp2/graphics/gui/widget/panel.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("panel", Panel);

Panel::Panel(P<Container> parent)
: Widget(parent)
{
    loadThemeData("panel");
}

void Panel::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    renderStretchedHV(window, layout.rect, 25.0f, t.background_image, t.background_color);
}

bool Panel::onPointerDown(io::Pointer::Button button, sf::Vector2f position, int id)
{
    return true;
}

};//!namespace gui
};//!namespace sp
