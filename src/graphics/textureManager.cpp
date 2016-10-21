#include <sp2/graphics/textureManager.h>
#include <sp2/io/lazyLoader.h>
#include <string.h>

namespace sp {

TextureManager textureManager;

__TextureManagerLoaderData* TextureManager::prepare(string name)
{
    __TextureManagerLoaderData* ptr = new __TextureManagerLoaderData();
    ptr->stream = io::ResourceProvider::get(name);
    LOG(Info, "Loading texture:", name);
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
        result->setSmooth(true);
        return result;
    }
    else
    {
        return loadFallback();
    }
}

sf::Texture* TextureManager::loadFallback()
{
    sf::Texture* texture = new sf::Texture();
    texture->create(32, 32);
    uint8_t pixels[32*32*sizeof(uint32_t)];
    memset(pixels, 0, sizeof(pixels));
    for(int x=0; x<32; x++)
        for(int y=0; y<32; y++)
            ((uint32_t*)pixels)[x + y * 32] = ((x / 4) % 2 == (y / 4) % 2) ? 0xFFFF00FF : 0x0000FFFF;
    texture->update(pixels);
    return texture;
}

};//!namespace sp
