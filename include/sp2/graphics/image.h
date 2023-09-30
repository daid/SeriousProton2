#ifndef SP2_GRAPHICS_IMAGE_H
#define SP2_GRAPHICS_IMAGE_H

#include <sp2/string.h>
#include <sp2/math/vector2.h>
#include <sp2/math/ray.h>
#include <sp2/math/rect.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/assert.h>


namespace sp {

template<typename T> class ImageBase
{
public:
    ImageBase() {}
    ImageBase(ImageBase<T>&& other) noexcept {
        pixels = std::move(other.pixels);
        size = other.size;
        other.size = Vector2i(0, 0);
    }
    ImageBase(Vector2i size): size(size) { pixels.resize(size.x * size.y); }
    ImageBase(Vector2i size, T color): size(size) { pixels.resize(size.x * size.y, color); }
    ImageBase(Vector2i size, std::vector<T>&& pixels): size(size) {
        sp2assert(static_cast<size_t>(size.x * size.y) == pixels.size(), "Given pixel buffer to image constructor must match image size.");
        this->pixels = std::move(pixels);
    }
    
    void operator=(ImageBase<T>&& other) noexcept {
        pixels = std::move(other.pixels);
        size = other.size;
        other.size = Vector2i(0, 0);
    }
    
    void update(Vector2i size, const T* ptr) {
        this->size = size;
        pixels.resize(size.x * size.y);
        memcpy(pixels.data(), ptr, size.x * size.y * sizeof(T));
    }
    void update(Vector2i size, const T* ptr, int pitch) {
        this->size = size;
        pixels.resize(size.x * size.y);
        for(int y=0; y<size.y; y++)
            memcpy(pixels.data() + size.x * y, ptr + pitch * y, size.x * sizeof(T));
    }
    
    void clear() {
        pixels.clear();
        size = Vector2i(0, 0);
    }
    
    Vector2i getSize() const { return size; }
    const T* getPtr() const { return &pixels[0]; }
    T* getPtr() { return &pixels[0]; }
    
    ImageBase<T> subImage(Rect2i area) const {
        area.shrinkToFitWithin(Rect2i(Vector2i(0, 0), size));
        ImageBase<T> result;
        result.update(area.size, pixels.data() + area.position.x + size.x * area.position.y, size.x);
        return result;
    }
    ImageBase<T> copy() const { ImageBase<T> result; result.update(size, pixels.data()); return result; }

    bool loadFromStream(io::ResourceStreamPtr stream) { return false; }
    bool loadFromFile(const string& filename) { return false; }
    
    //Write the image to a file. Supported extensions: png, bmp, tga, jpg, jpeg
    //Returns true on success.
    bool saveToFile(const string& filename) { return false; }
protected:
    Vector2i size;
    std::vector<T> pixels;
};

using Image = ImageBase<uint32_t>;
using Image8 = ImageBase<uint8_t>;

template<> bool Image::loadFromStream(io::ResourceStreamPtr stream);
template<> bool Image::loadFromFile(const string& filename);

//Write the image to a file. Supported extensions: png, bmp, tga, jpg, jpeg
//Returns true on success.
template<> bool Image::saveToFile(const string& filename);

}//namespace sp


#endif//SP2_GRAPHICS_IMAGE_H
