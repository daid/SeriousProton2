#include <sp2/graphics/image.h>
#include <sp2/graphics/image/hq2x.h>
#include <sp2/stringutil/convert.h>
#include <sp2/assert.h>


#define STBI_ASSERT(x) sp2assert(x, "stb_image assert")
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wshadow-compatible-local"
#endif//__GNUC__
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STBIWDEF static inline
#include "stb/stb_image_write.h"
#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvg/nanosvg.h"
#include "nanosvg/nanosvgrast.h"
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif//__GNUC__

namespace sp {

Image::Image()
{
}

Image::Image(Image&& other) noexcept
{
    pixels = std::move(other.pixels);
    size = other.size;
    other.size = Vector2i(0, 0);
}

Image::Image(Vector2i size)
: size(size)
{
    pixels.resize(size.x * size.y);
}

Image::Image(Vector2i size, uint32_t color)
: size(size)
{
    pixels.resize(size.x * size.y, color);
}

Image::Image(Vector2i size, std::vector<uint32_t>&& pixels)
: size(size)
{
    sp2assert(static_cast<size_t>(size.x * size.y) == pixels.size(), "Given pixel buffer to image constructor must match image size.");
    this->pixels = std::move(pixels);
}

void Image::operator=(Image&& other) noexcept
{
    pixels = std::move(other.pixels);
    size = other.size;
    other.size = Vector2i(0, 0);
}

void Image::update(Vector2i size, const uint32_t* ptr)
{
    this->size = size;
    pixels.resize(size.x * size.y);
    memcpy(pixels.data(), ptr, size.x * size.y * sizeof(uint32_t));
}

void Image::update(Vector2i size, const uint32_t* ptr, int pitch)
{
    this->size = size;
    pixels.resize(size.x * size.y);
    for(int y=0; y<size.y; y++)
        memcpy(pixels.data() + size.x * y, ptr + pitch * y, size.x * sizeof(uint32_t));
}

void Image::clear()
{
    pixels.clear();
    size = Vector2i(0, 0);
}

static int stream_read(void *user, char *data, int size)
{
    io::ResourceStream* stream = static_cast<io::ResourceStream*>(user);
    return stream->read(data, size);
}

static void stream_skip(void *user, int n)
{
    io::ResourceStream* stream = static_cast<io::ResourceStream*>(user);
    stream->seek(stream->tell() + n);
}

static int stream_eof(void *user)
{
    io::ResourceStream* stream = static_cast<io::ResourceStream*>(user);
    return stream->tell() == stream->getSize();
}

static stbi_io_callbacks stream_callbacks{
    .read = stream_read,
    .skip = stream_skip,
    .eof = stream_eof,
};

bool Image::loadFromStream(io::ResourceStreamPtr stream)
{
    if (!stream)
        return false;
    int x, y, channels;
    uint32_t* buffer = reinterpret_cast<uint32_t*>(stbi_load_from_callbacks(&stream_callbacks, stream.get(), &x, &y, &channels, 4));
    if (buffer)
    {
        update(Vector2i(x, y), buffer);
        stbi_image_free(buffer);
    }
    else
    {
        char header[5] = {0};
        stream->seek(0);
        stream->read(&header, sizeof(header));
        if (memcmp(header, "<svg", 4) == 0 || memcmp(header, "<?xml", 5) == 0)
        {
            stream->seek(0);
            string data = stream->readAll();
            auto svg = nsvgParse(const_cast<char*>(data.c_str()), "px", 96);
            float scale = 1.0;
            if (stream->hasFlag("scale"))
                scale = stringutil::convert::toFloat(stream->getFlag("scale"));
            x = std::ceil(svg->width * scale);
            y = std::ceil(svg->height * scale);
            struct NSVGrasterizer* rast = nsvgCreateRasterizer();
            buffer = new uint32_t[x*y];
        	nsvgRasterize(rast, svg, 0.0f, 0.0f, float(x) / svg->width, reinterpret_cast<unsigned char*>(buffer), x, y, x*sizeof(uint32_t));
            update(Vector2i(x, y), buffer);
            nsvgDeleteRasterizer(rast);
            nsvgDelete(svg);
        }
        else
        {
            return false;
        }
    }

    if (stream->hasFlag("hq2x") || stream->hasFlag("hq3x") || stream->hasFlag("hq4x"))
    {
        image::HQ2xConfig config;
        Vector2i tile_size(0, 0);
        config.scale = 2;
        if (stream->hasFlag("hq3x"))
            config.scale = 3;
        else if (stream->hasFlag("hq4x"))
            config.scale = 4;
        config.out_of_bounds = image::HQ2xConfig::OutOfBounds::Clamp;
        if (stream->hasFlag("wrap"))
            config.out_of_bounds = image::HQ2xConfig::OutOfBounds::Wrap;
        else if (stream->hasFlag("wrap"))
            config.out_of_bounds = image::HQ2xConfig::OutOfBounds::Transparent;
        if (stream->hasFlag("tiles"))
            tile_size.x = tile_size.y = stringutil::convert::toInt(stream->getFlag("tiles"));
        if (tile_size.x > 0 && tile_size.y > 0)
            image::hq2xTiles(*this, tile_size, config);
        else
            image::hq2x(*this, config);
    }
    return true;
}

bool Image::loadFromFile(const string& filename)
{
    int x, y, channels;
    uint32_t* buffer = reinterpret_cast<uint32_t*>(stbi_load(filename.c_str(), &x, &y, &channels, 4));
    if (buffer)
    {
        update(Vector2i(x, y), buffer);
        stbi_image_free(buffer);
        return true;
    }
    return false;
}

bool Image::saveToFile(const string& filename)
{
    string ext = filename.substr(filename.rfind(".") + 1).lower();
    
    if (ext == "png")
        return stbi_write_png(filename.c_str(), size.x, size.y, 4, pixels.data(), size.x * 4) != 0;
    if (ext == "bmp")
        return stbi_write_bmp(filename.c_str(), size.x, size.y, 4, pixels.data()) != 0;
    if (ext == "tga")
        return stbi_write_tga(filename.c_str(), size.x, size.y, 4, pixels.data()) != 0;
    if (ext == "jpg" || ext == "jpeg")
        return stbi_write_jpg(filename.c_str(), size.x, size.y, 4, pixels.data(), 90) != 0;

    return false;
}

#define PUT_PIXEL(_x, _y, _c) do { if ((_x) >= 0 && (_y) >= 0 && (_x) < size.x && (_y) < size.y) { pixels[(_x) + (_y) * size.x] = (_c); } } while(0)

void Image::draw(Ray2i ray, uint32_t color)
{
    if (ray.end.x < ray.start.x)
        std::swap(ray.start, ray.end);
    int dx = ray.end.x - ray.start.x;
    int dy = std::abs(ray.end.y - ray.start.y);
    bool steep = dx < dy;
    if (steep)
    {
        std::swap(ray.start.x, ray.start.y);
        std::swap(ray.end.x, ray.end.y);
        std::swap(dx, dy);
    }
    int sy = ray.start.y < ray.end.y ? 1 : -1;
    int err = 2*dy - dx;
    
    while(ray.start.x <= ray.end.x)
    {
        if (steep)
            PUT_PIXEL(ray.start.y, ray.start.x, color);
        else
            PUT_PIXEL(ray.start.x, ray.start.y, color);
        ray.start.x++;
        if (err > 0)
        {
            ray.start.y += sy;
            err -= 2*dx;
        }
        err += 2*dy;
    }
}

void Image::draw(Rect2i rect, uint32_t color)
{
    for(int x=0; x<rect.size.x; x++)
    {
        PUT_PIXEL(rect.position.x + x, rect.position.y, color);
        PUT_PIXEL(rect.position.x + x, rect.position.y + rect.size.y - 1, color);
    }
    for(int y=1; y<rect.size.y; y++)
    {
        PUT_PIXEL(rect.position.x, rect.position.y + y, color);
        PUT_PIXEL(rect.position.x + rect.size.x - 1, rect.position.y + y, color);
    }
}

void Image::draw(Vector2i position, const Image& img)
{
    if (position.x + img.size.x <= 0 || position.x > size.x)
        return;
    if (position.y + img.size.y <= 0 || position.y > size.y)
        return;
    int w = std::min(img.size.x, size.x - position.x);
    const uint32_t* src = img.pixels.data();
    uint32_t* dst = &pixels[position.x + position.y * size.x];
    if (position.x < 0)
    {
        src -= position.x;
        dst -= position.x;
        w += position.x;
    }
    int h = std::min(img.size.y, size.y - position.y);
    if (position.y < 0)
    {
        src -= position.y * img.size.x;
        dst -= position.y * size.x;
        h += position.y;
    }
    
    for(int y=0; y<h; y++)
    {
        memcpy(dst, src, w * sizeof(uint32_t));
        dst += size.x;
        src += img.size.x;
    }
}

void Image::drawFilled(Rect2i rect, uint32_t color)
{
    for(int y=0; y<rect.size.y; y++)
        for(int x=0; x<rect.size.x; x++)
            PUT_PIXEL(rect.position.x + x, rect.position.y + y, color);
}

void Image::drawCircle(Vector2i position, int radius, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius / 2);

    while (x >= y)
    {
        PUT_PIXEL(position.x + x, position.y + y, color);
        PUT_PIXEL(position.x + y, position.y + x, color);
        PUT_PIXEL(position.x - y, position.y + x, color);
        PUT_PIXEL(position.x - x, position.y + y, color);
        PUT_PIXEL(position.x - x, position.y - y, color);
        PUT_PIXEL(position.x - y, position.y - x, color);
        PUT_PIXEL(position.x + y, position.y - x, color);
        PUT_PIXEL(position.x + x, position.y - y, color);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void Image::drawFilledCircle(Vector2i position, int radius, uint32_t color)
{
    int x = radius-1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius / 2);

    while(x > 0)
    {
        for(int n=-x;n<=x; n++)
        {
            PUT_PIXEL(position.x + n, position.y + y, color);
            PUT_PIXEL(position.x + n, position.y - y, color);
        }

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }
        
        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void Image::drawFloodFill(Vector2i position, uint32_t color)
{
    if (position.x < 0 || position.y < 0 || position.x >= size.x || position.y >= size.y)
        return;
    uint32_t target_color = pixels[position.x + position.y * size.x];
    if (target_color == color)
        return;
    std::vector<Vector2i> todo_list;
    todo_list.push_back(position);
    while(todo_list.size() > 0)
    {
        position = todo_list.back();
        todo_list.pop_back();
        
        if (pixels[position.x + position.y * size.x] != target_color)
            continue;
        pixels[position.x + position.y * size.x] = color;
        if (position.x > 0)
            todo_list.emplace_back(position.x - 1, position.y);
        if (position.y > 0)
            todo_list.emplace_back(position.x, position.y - 1);
        if (position.x < size.x - 1)
            todo_list.emplace_back(position.x + 1, position.y);
        if (position.y < size.y - 1)
            todo_list.emplace_back(position.x, position.y + 1);
    }
}

void Image::drawFilledPolygon(const std::vector<Vector2i>& polygon, uint32_t color)
{
    if (polygon.size() < 3)
        return;
    int max = 0;
    int min = size.y;
    for(Vector2i p0 : polygon)
    {
        max = std::max(max, p0.y);
        min = std::min(min, p0.y);
    }
    for(int y=min; y<max; y++)
    {
        std::vector<int> nodes;
        Vector2i p0 = polygon.back();
        for(Vector2i p1 : polygon)
        {
            if ((p1.y < y && p0.y >= y) || (p0.y < y && p1.y >= y))
                nodes.push_back(p1.x + float(y-p1.y) / float(p0.y-p1.y) * float(p0.x-p1.x));
            p0 = p1;
        }
        
        std::sort(nodes.begin(), nodes.end());

        for(unsigned int n=0; n<nodes.size(); n+=2)
        {
            for(int x=nodes[n]; x<nodes[n+1]; x++)
                PUT_PIXEL(x, y, color);
        }
    }
}

Image Image::subImage(Rect2i area) const
{
    area.shrinkToFitWithin(Rect2i(Vector2i(0, 0), size));
    Image result;
    result.update(area.size, pixels.data() + area.position.x + size.x * area.position.y, size.x);
    return result;
}

}//namespace sp
