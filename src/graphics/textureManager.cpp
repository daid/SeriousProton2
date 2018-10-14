#include <sp2/graphics/textureManager.h>
#include <sp2/io/lazyLoader.h>
#include <sp2/graphics/image/hq2x.h>
#include <string.h>

#include <SFML/Graphics/Texture.hpp>

namespace sp {

TextureManager texture_manager;

class TextureManagerTexture : public Texture
{
public:
    TextureManagerTexture(string name)
    : Texture(Type::Static, name)
    {
        LOG(Info, "Loading texture:", name);
        //TODO: Fallback texture
        texture.setRepeated(true);
        texture.setSmooth(texture_manager.default_smooth);
    }

    void transferImageFromThread(std::shared_ptr<sp::Image> image)
    {
        if (name.find("#") > 0)
        {
            std::vector<string> keys = name.substr(name.find("#") + 1).lower().split(",");
            if (keys[0] == "hq2x" || keys[0] == "hq3x" || keys[0] == "hq4x")
            {
                sp::image::HQ2xConfig config;
                sp::Vector2i tile_size(0, 0);
                if (keys[0] == "hq2x")
                    config.scale = 2;
                else if (keys[0] == "hq3x")
                    config.scale = 3;
                else if (keys[0] == "hq4x")
                    config.scale = 4;
                config.out_of_bounds = sp::image::HQ2xConfig::OutOfBounds::Clamp;
                for(unsigned int n=1; n<keys.size(); n++)
                {
                    if (keys[n] == "clamp")
                        config.out_of_bounds = sp::image::HQ2xConfig::OutOfBounds::Clamp;
                    else if (keys[n] == "wrap")
                        config.out_of_bounds = sp::image::HQ2xConfig::OutOfBounds::Wrap;
                    else if (keys[n] == "transparent")
                        config.out_of_bounds = sp::image::HQ2xConfig::OutOfBounds::Transparent;
                    else if (keys[n].startswith("tiles="))
                        tile_size.x = tile_size.y = stringutil::convert::toInt(keys[n].substr(6));
                }
                if (tile_size.x > 0 && tile_size.y > 0)
                    sp::image::hq2xTiles(*image, tile_size, config);
                else
                    sp::image::hq2x(*image, config);
            }
        }

        std::lock_guard<std::mutex> lock(mutex);
        this->image = image;
    }
    
    virtual const sf::Texture* get() override
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (image)
        {
            LOG(Info, "Loaded image", name, image->getSize().x, "x", image->getSize().y);
            sf::Image tmp_image;
            tmp_image.create(image->getSize().x, image->getSize().y, (const uint8_t*)image->getPtr());
            if (!texture.loadFromImage(tmp_image))
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
    std::shared_ptr<sp::Image> image;
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
        std::shared_ptr<sp::Image> image = std::make_shared<sp::Image>();
        image->loadFromStream(stream);
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
