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

};//namespace sp
