#ifndef SP2_GRAPHICS_TEXTUREMANAGER_H
#define SP2_GRAPHICS_TEXTUREMANAGER_H

#include <sp2/io/lazyLoader.h>
#include <sp2/graphics/color.h>
#include <sp2/graphics/texture.h>

namespace sp {

class TextureManager : public io::LazyLoader<Texture*>
{
public:
    TextureManager();

    void setFallbackColors(sp::Color primary_color, sp::Color secondary_color);
    void setDefaultSmoothFiltering(bool enabled);
    bool isDefaultSmoothFiltering();
    
    //In case a texture is changed on disk, you can call this function to forcefully reload the texture.
    //Note that the loading still happens in the background, and the old texture will be shown till load is done.
    void forceRefresh(string name);
protected:
    virtual Texture* prepare(string name);
    virtual void backgroundLoader(Texture* texture, io::ResourceStreamPtr stream);
private:
    sp::Color fallback_primary_color;
    sp::Color fallback_secondary_color;
    
    bool default_smooth;
};
extern TextureManager texture_manager;

};//namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H
