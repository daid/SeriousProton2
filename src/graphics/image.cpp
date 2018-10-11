#include <sp2/graphics/image.h>
#include <sp2/assert.h>

#define STBI_ASSERT(x) sp2assert(x, "stb_image assert")
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#include "stb/stb_image.h"

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

void Image::update(Vector2i size, const uint32_t* ptr)
{
    this->size = size;
    pixels.resize(size.x * size.y);
    memcpy(pixels.data(), ptr, size.x * size.y * sizeof(uint32_t));
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

};//namespace sp
