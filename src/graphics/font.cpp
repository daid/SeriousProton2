#include <sp2/graphics/font.h>

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
    font = std::make_shared<sf::Font>();
    font_resource_stream = stream;
    LOG(Info, "Loading font:", name);
    if (!font->loadFromStream(*stream))
        LOG(Warning, "Failed to load font:", name);
    
}

std::shared_ptr<MeshData> Font::createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment)
{
    float size_scale = text_size / float(pixel_size);
    std::unordered_set<int>& known_glyphs = loaded_glyphs[pixel_size];
    bool texture_dirty = false;

    MeshData::Vertices vertices;
    vertices.reserve(s.size() * 6);
    sp::Vector2f position;
    int previous_character = 0;
    int line_count = 1;
    float max_line_width = 0;
    for(auto character : s)
    {
        // Apply the kerning offset
        position.x += font->getKerning(previous_character, character, pixel_size) * size_scale;
        previous_character = character;

        if (character == '\n')
        {
            position.x = 0;
            position.y -= font->getLineSpacing(pixel_size) * size_scale;
            line_count++;
            continue;
        }
        const sf::Glyph& glyph = font->getGlyph(character, pixel_size, false);
        if (known_glyphs.find(character) == known_glyphs.end())
        {
            known_glyphs.insert(character);
            texture_dirty = true;
        }

        if (character != ' ')
        {
            float u0 = glyph.textureRect.left;
            float v0 = glyph.textureRect.top;
            float u1 = glyph.textureRect.left + glyph.textureRect.width;
            float v1 = glyph.textureRect.top  + glyph.textureRect.height;
            
            float left = position.x + glyph.bounds.left * size_scale;
            float right = position.x + glyph.bounds.left * size_scale + glyph.bounds.width * size_scale;
            float top = position.y - glyph.bounds.top * size_scale;
            float bottom = position.y - glyph.bounds.top * size_scale - glyph.bounds.height * size_scale;

            sf::Vector3f p0(left, top, 0.0f);
            sf::Vector3f p1(right, top, 0.0f);
            sf::Vector3f p2(left, bottom, 0.0f);
            sf::Vector3f p3(right, bottom, 0.0f);

            vertices.emplace_back(p0, sp::Vector2f(u0, v0));
            vertices.emplace_back(p2, sp::Vector2f(u0, v1));
            vertices.emplace_back(p1, sp::Vector2f(u1, v0));
            vertices.emplace_back(p2, sp::Vector2f(u0, v1));
            vertices.emplace_back(p3, sp::Vector2f(u1, v1));
            vertices.emplace_back(p1, sp::Vector2f(u1, v0));
        }
        position.x += glyph.advance * size_scale;
        max_line_width = std::max(max_line_width, position.x);
    }
    sf::Vector2u texture_size = font->getTexture(pixel_size).getSize();
    for(auto& v : vertices)
    {
        v.uv[0] /= texture_size.x;
        v.uv[1] /= texture_size.y;
    }
    if (texture_dirty)
        static_cast<SfmlFontTexture*>(getTexture(pixel_size))->revision++;

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
        y_offset = area_size.y - font->getLineSpacing(pixel_size) * size_scale;
        break;
    case Alignment::Left:
    case Alignment::Center:
    case Alignment::Right:
        y_offset = (area_size.y - font->getLineSpacing(pixel_size) * size_scale * (line_count - 1)) / 2 - font->getLineSpacing(pixel_size) * size_scale * 0.3;
        break;
    case Alignment::BottomLeft:
    case Alignment::Bottom:
    case Alignment::BottomRight:
        y_offset = font->getLineSpacing(pixel_size) * size_scale * (line_count - 1);
        break;
    }
    for(auto& vertex : vertices)
    {
        vertex.position[0] += x_offset;
        vertex.position[1] += y_offset;
    }
    
    return std::make_shared<MeshData>(std::move(vertices));
}

Texture* Font::getTexture(int pixel_size)
{
    const auto& it = texture_cache.find(pixel_size);
    if (it != texture_cache.end())
        return it->second;
    Texture* texture = new SfmlFontTexture(font->getTexture(pixel_size), name + ":" + string(pixel_size));
    texture_cache[pixel_size] = texture;
    return texture;
}

};//!namespace sp
