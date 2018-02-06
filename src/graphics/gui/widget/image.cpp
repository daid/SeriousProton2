#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("image", Image);

Image::Image(P<Widget> parent)
: Widget(parent)
{
    loadThemeData("image");
}

void Image::setAttribute(const string& key, const string& value)
{
    if (key == "texture")
    {
        texture = value;
        markRenderDataOutdated();
    }
    else
    {
        Widget::setAttribute(key, value);
    }
}

void Image::updateRenderData()
{
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
    render_data.texture = textureManager.get(texture);
}

};//!namespace gui
};//!namespace sp
