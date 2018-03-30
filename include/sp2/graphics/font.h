#ifndef SP2_GRAPHICS_FONT_H
#define SP2_GRAPHICS_FONT_H

#include <sp2/nonCopyable.h>
#include <sp2/alignment.h>
#include <sp2/math/rect.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/texture.h>
#include <unordered_map>
#include <unordered_set>

namespace sp {

class Font : NonCopyable
{
public:
    /** Create a meshData for the given string.
        This potential updates the texture, which can invalidate all previous created MeshData objects for this pixel size.
        Compare the texture revision to the revision that it initially had to check if the data is still valid or needs to be re-generated.
     */
    virtual std::shared_ptr<MeshData> createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment) = 0;
    virtual Texture* getTexture(int pixel_size) = 0;
};

class FreetypeFontTexture;
class FreetypeFont : public Font
{
public:
    virtual std::shared_ptr<MeshData> createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment);
    virtual Texture* getTexture(int pixel_size);

private:
    class GlyphInfo
    {
    public:
        Rect2f uv_rect;
        Rect2f bounds;
        float advance;
    };

    FreetypeFont(string name, io::ResourceStreamPtr stream);
    ~FreetypeFont();

    string name;
    
    void* ft_library;
    void* ft_face;
    void* ft_stream_rec;
    
    //We need to keep the resource stream open, as the sf::Font instance keeps it open as well.
    //So we store the reference here.
    io::ResourceStreamPtr font_resource_stream;
    
    //Keep a cache of wrapped sf::Texture objects per font size
    std::unordered_map<int, FreetypeFontTexture*> texture_cache;
    //Keep track of glyphs that are loaded in the texture already.
    //As soon as we load a new glyph, the texture becomes invalid and needs to be updated.
    std::unordered_map<int, std::unordered_map<int, GlyphInfo>> loaded_glyphs;
    
    friend class FontManager;
    friend class FreetypeFontTexture;
};

class BitmapFont : public Font
{
public:
    virtual std::shared_ptr<MeshData> createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment);
    virtual Texture* getTexture(int pixel_size);

private:
    BitmapFont(string name, io::ResourceStreamPtr stream);

    string name;
    
    Texture* texture = nullptr;
    Vector2d glyph_size;
    std::unordered_map<int, Vector2d> glyphs;
    std::unordered_map<string, Rect2d> specials;
    
    friend class FontManager;
};

};//!namespace sp

#endif//SP2_GRAPHICS_FONT_H
