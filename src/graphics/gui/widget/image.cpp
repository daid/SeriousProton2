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

void Image::render(sf::RenderTarget& window)
{
    const sf::Texture* texture_ptr = textureManager.get(texture)->get();
    sf::Vector2f texture_size = sf::Vector2f(texture_ptr->getSize());
    sf::VertexArray a(sf::TrianglesStrip, 4);

    for(int n=0; n<4; n++)
        a[n].color = sf::Color::White;
    /*
    a[0].position = sf::Vector2f(getRect().left, getRect().top);
    a[1].position = sf::Vector2f(getRect().left, getRect().top + getRect().height);
    a[2].position = sf::Vector2f(getRect().left + getRect().width, getRect().top);
    a[3].position = sf::Vector2f(getRect().left + getRect().width, getRect().top + getRect().height);
    
    a[0].texCoords = sf::Vector2f(0, 0);
    a[1].texCoords = sf::Vector2f(0, texture_size.y);
    a[2].texCoords = sf::Vector2f(texture_size.x, 0);
    a[3].texCoords = sf::Vector2f(texture_size.x, texture_size.y);

    window.draw(a, texture_ptr);
    */
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
