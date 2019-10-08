#include <sp2/graphics/font/freetype.h>
#include <sp2/graphics/textureAtlas.h>
#include <sp2/stringutil/utf8.h>
#include <sp2/assert.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

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

FreetypeFont::FreetypeFont(string name, io::ResourceStreamPtr stream)
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
    if (ft_face) FT_Done_Face((FT_Face)ft_face);
    if (ft_stream_rec) delete (FT_StreamRec*)ft_stream_rec;
    if (ft_library) FT_Done_FreeType((FT_Library)ft_library);
}

Texture* FreetypeFont::getTexture(int pixel_size)
{
    const auto& it = texture_cache.find(pixel_size);
    if (it != texture_cache.end())
        return it->second;
    return nullptr;
}

bool FreetypeFont::getGlyphInfo(const char* str, int pixel_size, Font::GlyphInfo& info)
{
    std::unordered_map<int, GlyphInfo>& known_glyphs = loaded_glyphs[pixel_size];

    int consumed_characters;
    int unicode = stringutil::utf8::decodeSingle(str, &consumed_characters);

    if (known_glyphs.find(unicode) == known_glyphs.end())
    {
        FT_Face face = FT_Face(ft_face);
        
        GlyphInfo info;
        info.bounds = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.uv_rect = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.advance = 0;
        info.consumed_characters = consumed_characters;
        
        int glyph_index = FT_Get_Char_Index(face, unicode);
        if (glyph_index == 0 || FT_Load_Glyph(face, glyph_index, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) != 0)
        {
            LOG(Warning, "Failed to find glyph in font:", "0x" + string::hex(unicode));
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
                Image image(Vector2i(bitmap.width, bitmap.rows), 0xFFFFFFFF);
                if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
                {
                    sp2assert(false, "TODO");
                }
                else
                {
                    uint8_t* dst_pixels = (uint8_t*)image.getPtr();
                    for(unsigned int y=0; y<bitmap.rows; y++)
                    {
                        for(unsigned int x=0; x<bitmap.width; x++)
                            dst_pixels[(x + y * bitmap.width) * 4 + 3] = *src_pixels++;
                        src_pixels += bitmap.pitch - bitmap.width;
                    }
                }

                info.uv_rect = texture_cache[pixel_size]->add(std::move(image), 1);
                
                FT_Done_Glyph(glyph);
            }
        }
        
        known_glyphs[unicode] = info;
    }
    info = known_glyphs[unicode];
    return true;
}

float FreetypeFont::getLineSpacing(int pixel_size)
{
    if (((FT_Face)ft_face)->size->metrics.x_ppem != pixel_size)
    {
        FT_Set_Pixel_Sizes((FT_Face)ft_face, 0, pixel_size);
    }
    if (texture_cache.find(pixel_size) == texture_cache.end())
    {
        texture_cache[pixel_size] = new AtlasTexture(name, Vector2i(pixel_size * 16, pixel_size * 16));
    }
    return float(((FT_Face)ft_face)->size->metrics.height) / float(1 << 6);
}

float FreetypeFont::getBaseline(int pixel_size)
{
    return getLineSpacing(pixel_size) * 0.6;
}

float FreetypeFont::getKerning(const char* previous, const char* current)
{
    // Apply the kerning offset
    if (FT_HAS_KERNING((FT_Face)ft_face))
    {
        FT_Vector kerning;
        FT_Get_Kerning((FT_Face)ft_face, FT_Get_Char_Index((FT_Face)ft_face, *previous), FT_Get_Char_Index((FT_Face)ft_face, *current), FT_KERNING_DEFAULT, &kerning);
        if (!FT_IS_SCALABLE((FT_Face)ft_face))
            return float(kerning.x);
        else
            return float(kerning.x) / float(1 << 6);
    }
    return 0;
}

}//namespace sp
