#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/engine.h>
#include <SFML/Graphics.hpp>

namespace sp {
namespace gui {

SP_REGISTER_WIDGET("image", Image);

Image::Image(P<Container> parent)
: Widget(parent)
{
    loadThemeData("image");
}

void Image::setAttribute(const string& key, const string& value)
{
    if (key == "texture")
    {
        texture = value;
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
    
    a[0].position = sf::Vector2f(layout.rect.left, layout.rect.top);
    a[1].position = sf::Vector2f(layout.rect.left, layout.rect.top + layout.rect.height);
    a[2].position = sf::Vector2f(layout.rect.left + layout.rect.width, layout.rect.top);
    a[3].position = sf::Vector2f(layout.rect.left + layout.rect.width, layout.rect.top + layout.rect.height);
    
    a[0].texCoords = sf::Vector2f(0, 0);
    a[1].texCoords = sf::Vector2f(0, texture_size.y);
    a[2].texCoords = sf::Vector2f(texture_size.x, 0);
    a[3].texCoords = sf::Vector2f(texture_size.x, texture_size.y);

    window.draw(a, texture_ptr);
}

};//!namespace gui
};//!namespace sp
