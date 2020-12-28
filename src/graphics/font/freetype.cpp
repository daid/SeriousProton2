#include <sp2/graphics/font/freetype.h>
#include <sp2/graphics/textureAtlas.h>
#include <sp2/stringutil/utf8.h>
#include <sp2/assert.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif//__GNUC__
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#if defined(__GNUC__) && !defined(__clang__)
//#pragma GCC diagnostic pop
#endif//__GNUC__

static unsigned long ft_stream_read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    sp::io::ResourceStreamPtr& stream = *static_cast<sp::io::ResourceStreamPtr*>(rec->descriptor.pointer);
    if (stream->seek(offset) == int64_t(offset))
    {
        if (count > 0)
            return stream->read(reinterpret_cast<char*>(buffer), count);
        else
            return 0;
    }
    else
        return count > 0 ? 0 : 1; // error code is 0 if we're reading, or nonzero if we're seeking
}

void ft_stream_close(FT_Stream)
{
}

namespace sp {

FreetypeFont::FreetypeFont(const string& name, io::ResourceStreamPtr stream)
{
    ft_library = nullptr;
    ft_face = nullptr;

    font_resource_stream = stream;
    LOG(Info, "Loading font:", name);
    
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0)
    {
        LOG(Error, "Failed to initialize freetype library");
        return;
    }

    FT_StreamRec* stream_rec = new FT_StreamRec;
    memset(stream_rec, 0, sizeof(FT_StreamRec));
    stream_rec->base = nullptr;
    stream_rec->size = stream->getSize();
    stream_rec->pos = 0;
    stream_rec->descriptor.pointer = &font_resource_stream;
    stream_rec->read = &ft_stream_read;
    stream_rec->close = &ft_stream_close;

    // Setup the FreeType callbacks that will read our stream
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = stream_rec;
    args.driver = 0;

    // Load the new font face from the specified stream
    FT_Face face;
    if (FT_Open_Face(library, &args, 0, &face) != 0)
    {
        LOG(Error, "Failed to create font from stream:", name);
        FT_Done_FreeType(library);
        return;
    }

    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0)
    {
        LOG(Error, "Failed to select unicode for font:", name);
        FT_Done_Face(face);
        delete stream_rec;
        FT_Done_FreeType(library);
        return;
    }

    ft_library = library;
    ft_stream_rec = stream_rec;
    ft_face = face;
}

FreetypeFont::~FreetypeFont()
{
    if (ft_face) FT_Done_Face(static_cast<FT_Face>(ft_face));
    if (ft_stream_rec) delete static_cast<FT_StreamRec*>(ft_stream_rec);
    if (ft_library) FT_Done_FreeType(static_cast<FT_Library>(ft_library));
}

Texture* FreetypeFont::getTexture(int pixel_size)
{
    const auto& it = texture_cache.find(pixel_size);
    if (it != texture_cache.end())
        return it->second;
    return nullptr;
}

Font::CharacterInfo FreetypeFont::getCharacterInfo(const char* str)
{
    Font::CharacterInfo info;
    info.code = stringutil::utf8::decodeSingle(str, &info.consumed_bytes);
    return info;
}

bool FreetypeFont::getGlyphInfo(int char_code, int pixel_size, Font::GlyphInfo& info)
{
    std::unordered_map<int, GlyphInfo>& known_glyphs = loaded_glyphs[pixel_size];

    if (known_glyphs.find(char_code) == known_glyphs.end())
    {
        FT_Face face = static_cast<FT_Face>(ft_face);
        
        info.bounds = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.uv_rect = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.advance = 0;
        
        int glyph_index = FT_Get_Char_Index(face, char_code);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) != 0)
        {
            LOG(Warning, "Failed to find glyph in font:", "0x" + string::hex(char_code));
        }
        else
        {
            FT_Glyph glyph;
            if (FT_Get_Glyph(face->glyph, &glyph) == 0)
            {
                FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                FT_Bitmap& bitmap = FT_BitmapGlyph(glyph)->bitmap;
                
                info.advance = float(face->glyph->metrics.horiAdvance) / float(1 << 6);
                info.bounds.position.x = float(face->glyph->metrics.horiBearingX) / float(1 << 6);
                info.bounds.position.y = float(face->glyph->metrics.horiBearingY) / float(1 << 6);
                info.bounds.size.x = float(face->glyph->metrics.width) / float(1 << 6);
                info.bounds.size.y = float(face->glyph->metrics.height) / float(1 << 6);
                
                const uint8_t* src_pixels = bitmap.buffer;
                //We make a full white image, and then copy the alpha from the freetype render
                std::vector<uint32_t> image_pixels;
                image_pixels.resize(bitmap.width * bitmap.rows, 0xffffffff);
                if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    sp2assert(false, "TODO");
                }
                else
                {
                    uint8_t* dst_pixels = reinterpret_cast<uint8_t*>(image_pixels.data());
                    for(unsigned int y=0; y<bitmap.rows; y++)
                    {
                        for(unsigned int x=0; x<bitmap.width; x++)
                            dst_pixels[(x + y * bitmap.width) * 4 + 3] = *src_pixels++;
                        src_pixels += bitmap.pitch - bitmap.width;
                    }
                }
                Image image(Vector2i(bitmap.width, bitmap.rows), std::move(image_pixels));

                info.uv_rect = texture_cache[pixel_size]->add(std::move(image), 1);

                FT_Done_Glyph(glyph);
            }
        }
        
        known_glyphs[char_code] = info;
    }
    info = known_glyphs[char_code];
    return true;
}

float FreetypeFont::getLineSpacing(int pixel_size)
{
    if (static_cast<FT_Face>(ft_face)->size->metrics.x_ppem != pixel_size)
    {
        FT_Set_Pixel_Sizes(static_cast<FT_Face>(ft_face), 0, pixel_size);
    }
    if (texture_cache.find(pixel_size) == texture_cache.end())
    {
        texture_cache[pixel_size] = new AtlasTexture(name, Vector2i(pixel_size * 16, pixel_size * 16));
    }
    return float(static_cast<FT_Face>(ft_face)->size->metrics.height) / float(1 << 6);
}

float FreetypeFont::getBaseline(int pixel_size)
{
    return getLineSpacing(pixel_size) * 0.6;
}

float FreetypeFont::getKerning(int previous_char_code, int current_char_code)
{
    // Apply the kerning offset
    FT_Face face = static_cast<FT_Face>(ft_face);
    if (FT_HAS_KERNING(face))
    {
        FT_Vector kerning;
        FT_Get_Kerning(face, FT_Get_Char_Index(face, previous_char_code), FT_Get_Char_Index(face, current_char_code), FT_KERNING_DEFAULT, &kerning);
        if (!FT_IS_SCALABLE(face))
            return float(kerning.x);
        else
            return float(kerning.x) / float(1 << 6);
    }
    return 0;
}

}//namespace sp
