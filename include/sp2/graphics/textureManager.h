#ifndef SP2_GRAPHICS_TEXTUREMANAGER_H
#define SP2_GRAPHICS_TEXTUREMANAGER_H

#include <sp2/io/lazyLoader.h>
#include <sp2/graphics/color.h>
#include <SFML/Graphics/Texture.hpp>

namespace sp {

class __TextureManagerLoaderData
{
public:
    io::ResourceStreamPtr stream;
    sf::Image image;
};

class TextureManager : public io::LazyLoader<sf::Texture, __TextureManagerLoaderData>
{
public:
    TextureManager();

    void setFallbackColors(sp::Color primary_color, sp::Color secondary_color);
protected:
    virtual __TextureManagerLoaderData* prepare(string name);
    virtual __TextureManagerLoaderData* backgroundLoader(__TextureManagerLoaderData* ptr) ;
    virtual sf::Texture* finalize(__TextureManagerLoaderData* ptr);
    virtual sf::Texture* loadFallback();

private:
    sp::Color fallback_primary_color;
    sp::Color fallback_secondary_color;
};
extern TextureManager textureManager;

};//!namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H
