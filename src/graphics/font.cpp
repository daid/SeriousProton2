#include <sp2/graphics/font.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/stringutil/convert.h>

namespace sp {

class SfmlFontTexture : public Texture
{
public:
    SfmlFontTexture(const sf::Texture& texture, string name)
    : Texture(const_cast<sf::Texture&>(texture), Type::Dynamic, name)
    {
    }
    
    friend class Font;
};

Font::Font(string name, io::ResourceStreamPtr stream)
: name(name)
{
    if (name.endswith(".txt"))
    {
        Vector2d texture_size, glyph_size;
        std::vector<string> lines;
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
                bitmap_font = textureManager.get(value);
            }
            else if (key == "texture_size")
            {
                texture_size = stringutil::convert::toVector2d(value);
            }
            else if (key == "glyph_size")
            {
                glyph_size = stringutil::convert::toVector2d(value);
            }
            else if (key == "line")
            {
                lines.push_back(value);
            }
            else
            {
                LOG(Warning, "Ignoring line in bitmap font file:", line);
            }
        }
        bitmap_glyph_size = Vector2d(glyph_size.x / texture_size.x, glyph_size.y / texture_size.y);
        int y = 0;
        for(string line : lines)
        {
            int x = 0;
            for(int character : line)
            {
                bitmap_glyphs[character] = Vector2d(bitmap_glyph_size.x * x, bitmap_glyph_size.y * y);
                x++;
            }
            y++;
        }
    }
    else
    {
        font = std::make_shared<sf::Font>();
        font_resource_stream = stream;
        LOG(Info, "Loading font:", name);
        if (!font->loadFromStream(*stream))
            LOG(Warning, "Failed to load font:", name);
    }
}

std::shared_ptr<MeshData> Font::createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment)
{
    float size_scale = text_size / float(pixel_size);
    std::unordered_set<int>& known_glyphs = loaded_glyphs[pixel_size];
    bool texture_dirty = false;
    float line_spacing = text_size;
    sf::Glyph glyph;
    if (!font)
    {
        size_scale = 1.0;
        glyph.advance = text_size;
        glyph.bounds = sf::FloatRect(0, -text_size, text_size, text_size);
        glyph.textureRect = sf::IntRect(0, 0, 0, 0);
    }
    else
    {
        line_spacing = font->getLineSpacing(pixel_size) * size_scale;
    }

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    
    vertices.reserve(s.size() * 6);
    sp::Vector2f position;
    int previous_character = 0;
    int line_count = 1;
    float max_line_width = 0;
    for(auto character : s)
    {
        // Apply the kerning offset
        if (font)
            position.x += font->getKerning(previous_character, character, pixel_size) * size_scale;
        previous_character = character;

        if (character == '\n')
        {
            position.x = 0;
            position.y -= line_spacing;
            line_count++;
            continue;
        }
        if (font)
        {
            glyph = font->getGlyph(character, pixel_size, false);
            if (known_glyphs.find(character) == known_glyphs.end())
            {
                known_glyphs.insert(character);
                texture_dirty = true;
            }
        }

        if (character != ' ')
        {
            float u0 = glyph.textureRect.left;
            float v0 = glyph.textureRect.top;
            float u1 = glyph.textureRect.left + glyph.textureRect.width;
            float v1 = glyph.textureRect.top  + glyph.textureRect.height;
            if (!font)
            {
                auto it = bitmap_glyphs.find(character);
                if (it != bitmap_glyphs.end())
                {
                    u0 = it->second.x;
                    v0 = it->second.y;
                    u1 = u0 + bitmap_glyph_size.x;
                    v1 = v0 + bitmap_glyph_size.y;
                }
            }
            
            float left = position.x + glyph.bounds.left * size_scale;
            float right = position.x + glyph.bounds.left * size_scale + glyph.bounds.width * size_scale;
            float top = position.y - glyph.bounds.top * size_scale;
            float bottom = position.y - glyph.bounds.top * size_scale - glyph.bounds.height * size_scale;

            sf::Vector3f p0(left, top, 0.0f);
            sf::Vector3f p1(right, top, 0.0f);
            sf::Vector3f p2(left, bottom, 0.0f);
            sf::Vector3f p3(right, bottom, 0.0f);

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
        position.x += glyph.advance * size_scale;
        max_line_width = std::max(max_line_width, position.x);
    }
    
    if (font)
    {
        sf::Vector2u texture_size = font->getTexture(pixel_size).getSize();
        for(auto& v : vertices)
        {
            v.uv[0] /= texture_size.x;
            v.uv[1] /= texture_size.y;
        }
        if (texture_dirty)
            static_cast<SfmlFontTexture*>(getTexture(pixel_size))->revision++;
    }

    float x_offset = 0;
    float y_offset = 0;
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::BottomLeft:
    case Alignment::Left:
        x_offset = 0;
        break;
    case Alignment::Top:
    case Alignment::Center:
    case Alignment::Bottom:
        x_offset = (area_size.x - max_line_width) / 2;
        break;
    case Alignment::TopRight:
    case Alignment::Right:
    case Alignment::BottomRight:
        x_offset = area_size.x - max_line_width;
        break;
    }
    switch(alignment)
    {
    case Alignment::TopLeft:
    case Alignment::Top:
    case Alignment::TopRight:
        y_offset = area_size.y - line_spacing;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        y_offset = (area_size.y - line_spacing * (line_count - 1)) / 2;
        if (font)
            y_offset -= line_spacing * 0.3;
        else
            y_offset -= line_spacing * 0.5;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        y_offset = line_spacing * (line_count - 1);
        break;
    }
    for(auto& vertex : vertices)
    {
        vertex.position[0] += x_offset;
        vertex.position[1] += y_offset;
    }
    
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

Texture* Font::getTexture(int pixel_size)
{
    if (bitmap_font)
        return bitmap_font;

    const auto& it = texture_cache.find(pixel_size);
    if (it != texture_cache.end())
        return it->second;
    Texture* texture = new SfmlFontTexture(font->getTexture(pixel_size), name + ":" + string(pixel_size));
    texture_cache[pixel_size] = texture;
    return texture;
}

};//!namespace sp
