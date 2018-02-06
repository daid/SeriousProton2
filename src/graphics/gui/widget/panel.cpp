#include <sp2/graphics/gui/widget/panel.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("panel", Panel);

Panel::Panel(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("panel");
}

void Panel::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = createStretchedHV(getRenderSize(), 25.0f);
    render_data.texture = t.image;
    render_data.color = t.color;
}

bool Panel::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return true;
}

};//!namespace gui
};//!namespace sp
