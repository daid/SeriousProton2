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
    TextureManagerTexture(const string& name)
    : OpenGLTexture(Type::Static, name)
    {
        LOG(Info, "Loading texture:", name);
    }

    void transferImageFromThread(sp::Image&& image)
    {
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

bool TextureManager::isDefaultSmoothFiltering()
{
    return default_smooth;
}

Texture* TextureManager::prepare(const string& name)
{
    return new TextureManagerTexture(name);
}

void TextureManager::backgroundLoader(Texture* texture, io::ResourceStreamPtr stream)
{
    if (stream)
    {
        sp::Image image;
        if (!image.loadFromStream(stream))
            LOG(Warning, "Failed to load image:", texture->getName());
        (static_cast<TextureManagerTexture*>(texture))->transferImageFromThread(std::move(image));
    }
}

void TextureManager::setFallbackColors(sp::Color primary_color, sp::Color secondary_color)
{
    fallback_primary_color = primary_color;
    fallback_secondary_color = secondary_color;
}

void TextureManager::forceRefresh(const string& name)
{
    addToWorkqueue(get(name), name);
}

}//namespace sp
