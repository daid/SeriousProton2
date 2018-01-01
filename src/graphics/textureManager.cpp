#include <sp2/graphics/textureManager.h>
#include <sp2/io/lazyLoader.h>
#include <string.h>

namespace sp {

TextureManager textureManager;

TextureManager::TextureManager()
{
    fallback_primary_color = sp::Color(255, 255, 0);
    fallback_secondary_color = sp::Color(0, 0, 0, 0);
    
    default_smooth = true;
}

void TextureManager::setDefaultSmoothFiltering(bool enabled)
{
    default_smooth = enabled;
}

__TextureManagerLoaderData* TextureManager::prepare(string name)
{
    __TextureManagerLoaderData* ptr = new __TextureManagerLoaderData();
    ptr->stream = io::ResourceProvider::get(name);
    if (ptr->stream)
        LOG(Info, "Loading texture:", name);
    else
        LOG(Error, "Failed to open texture:", name);
    return ptr;
}

__TextureManagerLoaderData* TextureManager::backgroundLoader(__TextureManagerLoaderData* ptr)
{
    if (ptr->stream)
        ptr->image.loadFromStream(*ptr->stream);
    return ptr;
}

sf::Texture* TextureManager::finalize(__TextureManagerLoaderData* ptr)
{
    if (ptr->stream)
    {
        sf::Texture* result = new sf::Texture();
        result->loadFromImage(ptr->image);
        delete ptr;
        result->generateMipmap();
        result->setSmooth(default_smooth);
        result->setRepeated(true);
        return result;
    }
    else
    {
        return loadFallback();
    }
}

void TextureManager::setFallbackColors(sp::Color primary_color, sp::Color secondary_color)
{
    fallback_primary_color = primary_color;
    fallback_secondary_color = secondary_color;
}

sf::Texture* TextureManager::loadFallback()
{
    uint32_t a = fallback_primary_color.toInteger();
    uint32_t b = fallback_secondary_color.toInteger();
    
    sf::Texture* texture = new sf::Texture();
    texture->create(32, 32);
    uint8_t pixels[32*32*sizeof(uint32_t)];
    memset(pixels, 0, sizeof(pixels));
    for(int x=0; x<32; x++)
        for(int y=0; y<32; y++)
            ((uint32_t*)pixels)[x + y * 32] = ((x / 4) % 2 == (y / 4) % 2) ? a : b;
    texture->update(pixels);
    return texture;
}

};//!namespace sp
