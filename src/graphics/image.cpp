#include <sp2/graphics/image.h>
#include <sp2/assert.h>

#define STBI_ASSERT(x) sp2assert(x, "stb_image assert")
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STBIWDEF static inline
#include "stb/stb_image_write.h"

namespace sp {

Image::Image()
{
}

Image::Image(Image&& other)
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

void Image::operator=(Image&& other)
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

void Image::clear()
{
    pixels.clear();
    size = Vector2i(0, 0);
}

static int stream_read(void *user, char *data, int size)
{
    io::ResourceStream* stream = (io::ResourceStream*)user;
    return stream->read(data, size);
}

static void stream_skip(void *user, int n)
{
    io::ResourceStream* stream = (io::ResourceStream*)user;
    stream->seek(stream->tell() + n);
}

static int stream_eof(void *user)
{
    io::ResourceStream* stream = (io::ResourceStream*)user;
    return stream->tell() == stream->getSize();
}

static stbi_io_callbacks stream_callbacks{
    .read = stream_read,
    .skip = stream_skip,
    .eof = stream_eof,
};

bool Image::loadFromStream(io::ResourceStreamPtr stream)
{
    int x, y, channels;
    uint32_t* buffer = (uint32_t*)stbi_load_from_callbacks(&stream_callbacks, stream.get(), &x, &y, &channels, 4);
    if (buffer)
    {
        update(Vector2i(x, y), buffer);
        stbi_image_free(buffer);
        return true;
    }
    return false;
}

bool Image::saveToFile(sp::string filename)
{
    sp::string ext = filename.substr(filename.rfind(".") + 1).lower();
    
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

};//namespace sp
