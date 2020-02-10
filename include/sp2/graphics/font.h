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
    static constexpr int FlagClip = 0x02;
    static constexpr int FlagVertical = 0x03;

    /** Create a meshData for the given string.
        This potential updates the texture, which can invalidate all previous created MeshData objects for this pixel size.
        Compare the texture revision to the revision that it initially had to check if the data is still valid or needs to be re-generated.
     */
    std::shared_ptr<MeshData> createString(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment, int flags=0);
    virtual Texture* getTexture(int pixel_size) = 0;

    class PreparedFontString
    {
    public:
        class GlyphData
        {
        public:
            Vector2f position;
            int char_code;
            int string_offset;
            Vector3f normal;
        };
        std::vector<GlyphData> data;

        std::shared_ptr<MeshData> create();
        sp::Vector2f getUsedAreaSize() const;

    private:
        Font* font = nullptr;
        Alignment alignment;
        int pixel_size;
        float text_size;
        sp::Vector2d area_size;
        int flags;

        float getMaxLineWidth() const;
        int getLineCount() const;
        int lastLineCharacterCount() const;

        void alignAll();

        friend class Font;
    };
    PreparedFontString prepare(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment, int flags=0);

protected:
    class CharacterInfo
    {
    public:
        int code;
        int consumed_bytes;
    };
    class GlyphInfo
    {
    public:
        Rect2f uv_rect;
        Rect2f bounds;
        float advance;
    };
    virtual CharacterInfo getCharacterInfo(const char* str) = 0;
    virtual bool getGlyphInfo(int char_code, int pixel_size, GlyphInfo& info) = 0;
    virtual float getLineSpacing(int pixel_size) = 0;
    virtual float getBaseline(int pixel_size) = 0;
    virtual float getKerning(int previous_char_code, int current_char_code) = 0;
};

}//namespace sp

#endif//SP2_GRAPHICS_FONT_H
