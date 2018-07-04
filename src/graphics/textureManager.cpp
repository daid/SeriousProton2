#include <sp2/graphics/textureManager.h>
#include <sp2/io/lazyLoader.h>
#include <string.h>

namespace sp {

TextureManager texture_manager;

class TextureManagerTexture : public Texture
{
public:
    TextureManagerTexture(string resource_name)
    : Texture(Type::Static, resource_name)
    {
        LOG(Info, "Loading texture:", resource_name);
        //TODO: Fallback texture
        texture.setRepeated(true);
        texture.setSmooth(texture_manager.default_smooth);
    }

    void transferImageFromThread(std::shared_ptr<sf::Image> image)
    {
        std::lock_guard<std::mutex> lock(mutex);
        this->image = image;
    }
    
    virtual const sf::Texture* get() override
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (image)
        {
            LOG(Info, "Loaded image", name, image->getSize().x, "x", image->getSize().y);
            if (!texture.loadFromImage(*image))
            {
                LOG(Warning, "loadFromImage failed for", name);
            }
            image = nullptr;
            revision++;
        }
        return &texture;
    }
private:
    std::mutex mutex;

    sf::Texture texture;
    std::shared_ptr<sf::Image> image;
};

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

Texture* TextureManager::prepare(string name)
{
    return new TextureManagerTexture(name);
}

void TextureManager::backgroundLoader(Texture* texture, io::ResourceStreamPtr stream)
{
    if (stream)
    {
        std::shared_ptr<sf::Image> image = std::make_shared<sf::Image>();
        image->loadFromStream(*stream);
        (static_cast<TextureManagerTexture*>(texture))->transferImageFromThread(image);
    }
}

void TextureManager::setFallbackColors(sp::Color primary_color, sp::Color secondary_color)
{
    fallback_primary_color = primary_color;
    fallback_secondary_color = secondary_color;
}

/*
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
*/

};//namespace sp
