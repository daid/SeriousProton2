#include <sp2/graphics/font.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/textureAtlas.h>
#include <sp2/graphics/opengl.h>
#include <sp2/stringutil/convert.h>
#include <sp2/assert.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H


namespace sp {

std::shared_ptr<MeshData> Font::createString(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment)
{
    return prepare(s, pixel_size, text_size, area_size, alignment).create();
}

Font::PreparedFontString Font::prepare(const string& s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment)
{
    float size_scale = text_size / float(pixel_size);
    float line_spacing = getLineSpacing(pixel_size) * size_scale;
    PreparedFontString result;

    result.font = this;
    result.s = s;
    result.alignment = alignment;
    result.text_size = text_size;
    result.pixel_size = pixel_size;
    result.max_line_width = 0;
    result.line_count = 1;
    result.area_size = area_size;

    sp::Vector2f position(0, -line_spacing);
    int previous_character_index = -1;
    float current_line_width = 0;
    unsigned int line_start_result_index = 0;
    
    for(unsigned int index=0; index<s.size(); )
    {
        if (previous_character_index > -1)
        {
            position.x += getKerning(&s[previous_character_index], &s[index]) * size_scale;
        }
        previous_character_index = index;

        PreparedFontString::GlyphData data;
        data.position = position;
        data.string_offset = index;
        result.data.push_back(data);

        if (s[index] == '\n')
        {
            position.x = 0;
            position.y -= line_spacing;
            result.line_count++;
            result.alignLine(line_start_result_index, current_line_width);
            result.max_line_width = std::max(result.max_line_width, current_line_width);
            current_line_width = 0;
            line_start_result_index = result.data.size();
            index += 1;
            continue;
        }
        
        GlyphInfo glyph;
        if (!getGlyphInfo(&s[index], pixel_size, glyph))
        {
            glyph.consumed_characters = 1;
            glyph.advance = 0;
            glyph.bounds.size.x = 0;
        }

        current_line_width = std::max(current_line_width, position.x + (glyph.bounds.position.x + glyph.bounds.size.x) * size_scale);
        position.x += glyph.advance * size_scale;
        index += glyph.consumed_characters;
    }

    result.alignLine(line_start_result_index, current_line_width);
    result.max_line_width = std::max(result.max_line_width, current_line_width);
    
    result.alignAll();
    
    return result;
}

void Font::PreparedFontString::alignAll()
{
    float size_scale = text_size / float(pixel_size);
    float line_spacing = font->getLineSpacing(pixel_size) * size_scale;

    sp::Vector2f offset;
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        offset.x = 0;
        break;
    case Alignment::Top:
    case Alignment::Center:
    case Alignment::Bottom:
        offset.x = (area_size.x - max_line_width) / 2;
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        offset.x = area_size.x - max_line_width;
        break;
    }
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        offset.y = area_size.y;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        offset.y = (area_size.y + line_spacing * (line_count + 1)) / 2;
        offset.y -= font->getBaseline(pixel_size) * size_scale * 0.5;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        offset.y = line_spacing * line_count;
        break;
    }
    for(GlyphData& d : data)
    {
        d.position += offset;
    }
}

sp::Vector2f Font::PreparedFontString::getUsedAreaSize()
{
    return sp::Vector2f(max_line_width, (float(line_count) + 0.3) * font->getLineSpacing(pixel_size) * text_size / float(pixel_size));
}

std::shared_ptr<MeshData> Font::PreparedFontString::create(bool clip)
{
    float size_scale = text_size / float(pixel_size);

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    
    vertices.reserve(data.size() * 6);
    indices.reserve(data.size() * 4);

    for(const GlyphData& d : data)
    {
        GlyphInfo glyph;
        if (d.string_offset < 0)
        {
            char tmp[2] = {char(-d.string_offset), 0};
            if (!font->getGlyphInfo(tmp, pixel_size, glyph))
            {
                glyph.consumed_characters = 1;
                glyph.advance = 0;
                glyph.bounds.size.x = 0;
            }
        }
        else
        {
            if (!font->getGlyphInfo(&s[d.string_offset], pixel_size, glyph))
            {
                glyph.consumed_characters = 1;
                glyph.advance = 0;
                glyph.bounds.size.x = 0;
            }
        }

        if (glyph.bounds.size.x > 0.0)
        {
            float u0 = glyph.uv_rect.position.x;
            float v0 = glyph.uv_rect.position.y;
            float u1 = glyph.uv_rect.position.x + glyph.uv_rect.size.x;
            float v1 = glyph.uv_rect.position.y + glyph.uv_rect.size.y;
            
            float left = d.position.x + glyph.bounds.position.x * size_scale;
            float right = left + glyph.bounds.size.x * size_scale;
            float top = d.position.y + glyph.bounds.position.y * size_scale;
            float bottom = top - glyph.bounds.size.y * size_scale;
            
            if (clip)
            {
                if (right < 0)
                    continue;
                if (left < 0)
                {
                    u0 = u1 - glyph.uv_rect.size.x * (0 - right) / (left - right);
                    left = 0;
                }

                if (left > area_size.x)
                    continue;
                if (right > area_size.x)
                {
                    u1 = u0 + glyph.uv_rect.size.x * (area_size.x - left) / (right - left);
                    right = area_size.x;
                }

                if (top < 0)
                    continue;
                if (bottom < 0)
                {
                    v1 = v0 + glyph.uv_rect.size.y * (0 - top) / (bottom - top);
                    bottom = 0;
                }

                if (bottom > area_size.y)
                    continue;
                if (top > area_size.y)
                {
                    v0 = v1 - glyph.uv_rect.size.y * (area_size.y - bottom) / (top - bottom);
                    top = area_size.y;
                }
            }

            Vector3f p0(left, top, 0.0f);
            Vector3f p1(right, top, 0.0f);
            Vector3f p2(left, bottom, 0.0f);
            Vector3f p3(right, bottom, 0.0f);

            indices.emplace_back(vertices.size() + 0);
            indices.emplace_back(vertices.size() + 2);
            indices.emplace_back(vertices.size() + 1);
            indices.emplace_back(vertices.size() + 2);
            indices.emplace_back(vertices.size() + 3);
            indices.emplace_back(vertices.size() + 1);

            vertices.emplace_back(p0, sp::Vector2f(u0, v0));
            vertices.emplace_back(p1, sp::Vector2f(u1, v0));
            vertices.emplace_back(p2, sp::Vector2f(u0, v1));
            vertices.emplace_back(p3, sp::Vector2f(u1, v1));
        }
    }

    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

void Font::PreparedFontString::alignLine(unsigned int line_start_result_index, float current_line_width)
{
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        break;
    case Alignment::Center:
    case Alignment::Top:
    case Alignment::Bottom:
        if (current_line_width < max_line_width)
        {
            float offset = (max_line_width - current_line_width) / 2.0;
            for(unsigned int n=line_start_result_index; n<data.size(); n++)
                data[n].position.x += offset;
        }
        else
        {
            float offset = (current_line_width - max_line_width) / 2.0;
            for(unsigned int n=0; n<line_start_result_index; n++)
                data[n].position.x += offset;
        }
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        if (current_line_width < max_line_width)
        {
            float offset = max_line_width - current_line_width;
            for(unsigned int n=line_start_result_index; n<data.size(); n++)
                data[n].position.x += offset;
        }
        else
        {
            float offset = current_line_width - max_line_width;
            for(unsigned int n=0; n<line_start_result_index; n++)
                data[n].position.x += offset;
        }
        break;
    }
}

static unsigned long ft_stream_read(FT_Stream rec, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    io::ResourceStreamPtr& stream = *static_cast<io::ResourceStreamPtr*>(rec->descriptor.pointer);
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

    if (known_glyphs.find(*str) == known_glyphs.end())
    {
        FT_Face face = FT_Face(ft_face);
        
        GlyphInfo info;
        info.bounds = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.uv_rect = Rect2f(Vector2f(0, 0), Vector2f(0, 0));
        info.advance = 0;
        info.consumed_characters = 1;
        
        int glyph_index = FT_Get_Char_Index(face, *str);
        if (glyph_index != 0 && FT_Load_Glyph(face, glyph_index, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) == 0)
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
        
        known_glyphs[*str] = info;
    }
    info = known_glyphs[*str];
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

BitmapFont::BitmapFont(string name, io::ResourceStreamPtr stream)
: name(name)
{
    Vector2f texture_size, pixel_glyph_size;
    std::vector<string> lines;
    std::vector<string> special_lines;
    glyph_advance = sp::Vector2f(1, 1);
    while(stream->tell() != stream->getSize())
    {
        string line = stream->readLine();
        int seperator = line.find(":");
        if (seperator < 0)
        {
            if (line.strip() != "") LOG(Warning, "Ignoring line in bitmap font file:", line);
            continue;
        }
        string key = line.substr(0, seperator).strip().lower();
        string value = line.substr(seperator + 1).strip();
        if (key == "texture")
        {
            texture = texture_manager.get(value);
        }
        else if (key == "texture_size")
        {
            texture_size = stringutil::convert::toVector2f(value);
        }
        else if (key == "glyph_size")
        {
            pixel_glyph_size = stringutil::convert::toVector2f(value);
        }
        else if (key == "line")
        {
            lines.push_back(value);
        }
        else if (key == "special")
        {
            special_lines.push_back(value);
        }
        else if (key == "advance")
        {
            glyph_advance = stringutil::convert::toVector2f(value);
        }
        else
        {
            LOG(Warning, "Ignoring line in bitmap font file:", line);
        }
    }
    glyph_size = Vector2f(pixel_glyph_size.x / texture_size.x, pixel_glyph_size.y / texture_size.y);
    int y = 0;
    for(string line : lines)
    {
        int x = 0;
        for(int character : line)
        {
            glyphs[character] = Vector2f(glyph_size.x * x, glyph_size.y * y);
            x++;
        }
        y++;
    }
    for(string line : special_lines)
    {
        std::vector<string> parts = line.split(",", 3);
        if (parts.size() != 4)
        {
            LOG(Warning, "Ignoring malformed special line in font:", line);
            continue;
        }
        int x = stringutil::convert::toInt(parts[0].strip());
        int y = stringutil::convert::toInt(parts[1].strip());
        int w = stringutil::convert::toInt(parts[2].strip());
        
        specials[parts[3].strip()] = Rect2f(Vector2f(glyph_size.x * x, glyph_size.y * y), Vector2f(glyph_size.x * w, glyph_size.y));
    }
}

bool BitmapFont::getGlyphInfo(const char* str, int pixel_size, GlyphInfo& info)
{
    for(auto it : specials)
    {
        if (strncmp(it.first.c_str(), str, it.first.length()) == 0)
        {
            info.uv_rect = it.second;
            info.bounds = {0, pixel_size * it.second.size.y / glyph_size.y, pixel_size * it.second.size.x / glyph_size.x, pixel_size * it.second.size.y / glyph_size.y};
            info.advance = pixel_size * glyph_advance.x * it.second.size.x / glyph_size.x;
            info.consumed_characters = it.first.length();
            return true;
        }
    }
    if (*str == ' ')
    {
        info.uv_rect = {0, 0, 0, 0};
        info.bounds = {0, 0, 0, 0};
        info.advance = pixel_size * glyph_advance.x;
        info.consumed_characters = 1;
        return true;
    }
    auto it = glyphs.find(*str);
    if (it == glyphs.end())
        return false;
    info.uv_rect = {it->second.x, it->second.y, glyph_size.x, glyph_size.y};
    info.bounds = {0, float(pixel_size), float(pixel_size), float(pixel_size)};
    info.advance = pixel_size * glyph_advance.x;
    info.consumed_characters = 1;
    return true;
}

float BitmapFont::getLineSpacing(int pixel_size)
{
    return glyph_advance.y * pixel_size;
}

float BitmapFont::getBaseline(int pixel_size)
{
    return getLineSpacing(pixel_size);
}

float BitmapFont::getKerning(const char* previous, const char* current)
{
    return 0;
}

Texture* BitmapFont::getTexture(int pixel_size)
{
    return texture;
}

};//namespace sp
