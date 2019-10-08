#ifndef SP2_GRAPHICS_FONT_FREETYPE_H
#define SP2_GRAPHICS_FONT_FREETYPE_H

#include <sp2/graphics/font.h>

namespace sp {

class AtlasTexture;
class FreetypeFont : public Font
{
public:
    FreetypeFont(string name, io::ResourceStreamPtr stream);
    ~FreetypeFont();

    virtual Texture* getTexture(int pixel_size) override;
protected:
    virtual bool getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info) override;
    virtual float getLineSpacing(int pixel_size) override;
    virtual float getBaseline(int pixel_size) override;
    virtual float getKerning(const char* previous, const char* current) override;

private:
    string name;
    
    void* ft_library;
    void* ft_face;
    void* ft_stream_rec;
    
    //We need to keep the resource stream open, as the freetype keeps it open as well.
    //So we store the reference here.
    io::ResourceStreamPtr font_resource_stream;
    
    //Keep a cache of wrapped Texture objects per font size
    std::unordered_map<int, AtlasTexture*> texture_cache;
    //Keep track of glyphs that are loaded in the texture already.
    //As soon as we load a new glyph, the texture becomes invalid and needs to be updated.
    std::unordered_map<int, std::unordered_map<int, GlyphInfo>> loaded_glyphs;
};

}//namespace sp

#endif//SP2_GRAPHICS_FONT_FREETYPE_H
