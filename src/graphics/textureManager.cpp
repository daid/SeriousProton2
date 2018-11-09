#include <sp2/graphics/textureManager.h>
#include <sp2/io/lazyLoader.h>
#include <sp2/graphics/image/hq2x.h>
#include <sp2/graphics/opengl.h>
#include <string.h>

namespace sp {

TextureManager texture_manager;

class TextureManagerTexture : public OpenGLTexture
{
public:
    TextureManagerTexture(string name)
    : OpenGLTexture(Type::Static, name)
    {
        LOG(Info, "Loading texture:", name);
    }

    void transferImageFromThread(sp::Image&& image)
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
                    sp::image::hq2xTiles(image, tile_size, config);
                else
                    sp::image::hq2x(image, config);
            }
        }

        setImage(std::move(image));
    }
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
        sp::Image image;
        image.loadFromStream(stream);
        (static_cast<TextureManagerTexture*>(texture))->transferImageFromThread(std::move(image));
    }
}

void TextureManager::setFallbackColors(sp::Color primary_color, sp::Color secondary_color)
{
    fallback_primary_color = primary_color;
    fallback_secondary_color = secondary_color;
}

void TextureManager::forceRefresh(string name)
{
    addToWorkqueue(get(name), name);
}

};//namespace sp
