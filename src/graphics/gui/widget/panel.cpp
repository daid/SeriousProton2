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

void Panel::render(sf::RenderTarget& window)
{
    const ThemeData::StateData& t = theme->states[int(getState())];
    //renderStretchedHV(window, getRect(), 25.0f, t.image, t.color);
}

void Panel::updateRenderData()
{
    const ThemeData::StateData& t = theme->states[int(getState())];

    MeshData::Vertices vertices;
    sp::Vector2d p0(0, 0);
    sp::Vector2d p1(getRenderSize());
    vertices.emplace_back(sf::Vector3f(p0.x, p0.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f(p1.x, p0.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(p0.x, p1.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(p0.x, p1.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(p1.x, p0.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(p1.x, p1.y, 0.0f), sp::Vector2f(1, 0));

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.mesh = MeshData::create(std::move(vertices), MeshData::Type::Dynamic);
    render_data.texture = t.image;
    render_data.color = t.color;
}

bool Panel::onPointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    return true;
}

};//!namespace gui
};//!namespace sp
