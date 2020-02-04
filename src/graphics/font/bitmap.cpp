#include <sp2/graphics/font/bitmap.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/stringutil/convert.h>


namespace sp {

BitmapFont::BitmapFont(const string& name, io::ResourceStreamPtr stream)
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
    aspect_ratio = pixel_glyph_size.x / pixel_glyph_size.y;
    glyph_advance.x *= aspect_ratio;
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

        int code = specials.size() | special_mask | (w << 16);

        glyphs[code] = Vector2f(glyph_size.x * x, glyph_size.y * y);
        specials[parts[3].strip()] = code;
    }
}

Font::CharacterInfo BitmapFont::getCharacterInfo(const char* str)
{
    Font::CharacterInfo info;

    for(auto it : specials)
    {
        if (strncmp(it.first.c_str(), str, it.first.length()) == 0)
        {
            info.code = it.second;
            info.consumed_bytes = it.first.length();
            return info;
        }
    }
    info.code = *str;
    info.consumed_bytes = 1;
    return info;
}

bool BitmapFont::getGlyphInfo(int char_code, int pixel_size, GlyphInfo& info)
{
    if (char_code == ' ')
    {
        info.uv_rect = {0, 0, 0, 0};
        info.bounds = {0, 0, 0, 0};
        info.advance = pixel_size * glyph_advance.x;
        return true;
    }
    auto it = glyphs.find(char_code);
    if (it == glyphs.end())
        return false;
    float w = 1.0f;
    if (char_code & special_mask)
    {
        w = float(char_code >> 16);
    }
    info.uv_rect = {it->second.x, it->second.y, glyph_size.x * w, glyph_size.y};
    info.bounds = {0, float(pixel_size), float(pixel_size) * aspect_ratio * w, float(pixel_size)};
    info.advance = pixel_size * glyph_advance.x * w;
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

float BitmapFont::getKerning(int previous_char_code, int current_char_code)
{
    return 0.0f;
}

Texture* BitmapFont::getTexture(int pixel_size)
{
    return texture;
}

}//namespace sp
