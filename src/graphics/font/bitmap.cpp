#include <sp2/graphics/font/bitmap.h>
#include <sp2/graphics/textureManager.h>


namespace sp {

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
