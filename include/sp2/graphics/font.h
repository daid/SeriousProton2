#ifndef SP2_GRAPHICS_FONT_H
#define SP2_GRAPHICS_FONT_H

#include <sp2/nonCopyable.h>
#include <sp2/alignment.h>
#include <sp2/math/rect.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/texture.h>

namespace sp {

class Font : NonCopyable
{
public:
    static constexpr int FlagLineWrap = 0x01;

    /** Create a meshData for the given string.
        This potential updates the texture, which can invalidate all previous created MeshData objects for this pixel size.
        Compare the texture revision to the revision that it initially had to check if the data is still valid or needs to be re-generated.
     */
    std::shared_ptr<MeshData> createString(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment, int flags);
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
    PreparedFontString prepare(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment, int flags);
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

}//namespace sp

#endif//SP2_GRAPHICS_FONT_H
