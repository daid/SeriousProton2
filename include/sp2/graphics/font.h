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
    std::shared_ptr<MeshData> createString(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment);
    virtual Texture* getTexture(int pixel_size) = 0;

    class PreparedFontString
    {
    public:
        class GlyphData
        {
        public:
            sp::Vector2f position;
            int string_offset;
        };
        std::vector<GlyphData> data;
        
        std::shared_ptr<MeshData> create(bool clip=false);
        sp::Vector2f getUsedAreaSize();
    private:
        Font* font;
        string s;
        Alignment alignment;
        int pixel_size;
        float text_size;
        float max_line_width;
        int line_count;
        sp::Vector2d area_size;
        
        void alignLine(unsigned int line_start_result_index, float current_line_width);
        void alignAll();
        
        friend class Font;
    };
    PreparedFontString prepare(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment);
protected:
    class GlyphInfo
    {
    public:
        Rect2f uv_rect;
        Rect2f bounds;
        float advance;
        int consumed_characters;
    };
    virtual bool getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info) = 0;
    virtual float getLineSpacing(int pixel_size) = 0;
    virtual float getBaseline(int pixel_size) = 0;
    virtual float getKerning(const char* previous, const char* current) = 0;
};


class AtlasTexture;
class FreetypeFont : public Font
{
public:
    FreetypeFont(string name, io::ResourceStreamPtr stream);
    ~FreetypeFont();

    virtual Texture* getTexture(int pixel_size);
protected:
    virtual bool getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info);
    virtual float getLineSpacing(int pixel_size);
    virtual float getBaseline(int pixel_size);
    virtual float getKerning(const char* previous, const char* current);

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
    
    friend class AtlasTexture;
};

class BitmapFont : public Font
{
public:
    BitmapFont(string name, io::ResourceStreamPtr stream);

    virtual Texture* getTexture(int pixel_size);

protected:
    virtual bool getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info);
    virtual float getLineSpacing(int pixel_size);
    virtual float getBaseline(int pixel_size);
    virtual float getKerning(const char* previous, const char* current);

private:
    string name;
    
    Texture* texture = nullptr;
    Vector2f glyph_size;
    Vector2f glyph_advance;
    std::unordered_map<int, Vector2f> glyphs;
    std::unordered_map<string, Rect2f> specials;
};


};//namespace sp

#endif//SP2_GRAPHICS_FONT_H
