#ifndef SP2_GRAPHICS_FONT_BITMAP_H
#define SP2_GRAPHICS_FONT_BITMAP_H

#include <sp2/graphics/font.h>


namespace sp {

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

#endif//SP2_GRAPHICS_FONT_BITMAP_H
