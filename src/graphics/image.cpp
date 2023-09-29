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

template<> bool Image::loadFromStream(io::ResourceStreamPtr stream)
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

template<> bool Image::loadFromFile(const string& filename)
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

template<> bool Image::saveToFile(const string& filename)
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

}//namespace sp
