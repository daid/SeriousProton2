#include <sp2/graphics/textureManager.h>
#include <sp2/io/resourceProvider.h>
#include <string.h>

namespace sp {

sf::Texture* TextureManager::fallback_texture;
std::map<string, sf::Texture*> TextureManager::textures;

sf::Texture* TextureManager::get(string name)
{
    auto it = textures.find(name);
    if (it != textures.end())
        return it->second;

    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    if (!stream) stream = io::ResourceProvider::get(name + ".png");
    if (stream)
    {
        sf::Image image;
        if (image.loadFromStream(*stream))
        {
            sf::Texture* texture = new sf::Texture();
            texture->loadFromImage(image);
            textures[name] = texture;
            return texture;
        }
    }

    if (!fallback_texture)
    {
        fallback_texture = new sf::Texture();
        fallback_texture->create(32, 32);
        uint8_t pixels[32*32*4];
        memset(pixels, 0, sizeof(pixels));
        for(int x=0; x<32; x++)
            for(int y=0; y<32; y++)
                ((uint32_t*)pixels)[x + y * 32] = ((x / 4) % 2 == (y / 4) % 2) ? 0xFFFF00FF : 0x0000FFFF;
        fallback_texture->update(pixels);
    }
    textures[name] = fallback_texture;
    return fallback_texture;
}

};//!namespace sp
