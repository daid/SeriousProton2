#include <sp2/graphics/font.h>


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

};//namespace sp
