#ifndef SP2_GRAPHICS_IMAGE_H
#define SP2_GRAPHICS_IMAGE_H

#include <sp2/string.h>
#include <sp2/math/vector2.h>
#include <sp2/math/ray.h>
#include <sp2/math/rect.h>
#include <sp2/io/resourceProvider.h>


namespace sp {

class Image
{
public:
    Image();
    Image(Image&& other) noexcept;
    Image(Vector2i size);
    Image(Vector2i size, uint32_t color);
    Image(Vector2i size, std::vector<uint32_t>&& pixels);
    
    void operator=(Image&& other) noexcept;
    
    void update(Vector2i size, const uint32_t* ptr);
    void update(Vector2i size, const uint32_t* ptr, int pitch);
    bool loadFromStream(io::ResourceStreamPtr stream);
    bool loadFromFile(const string& filename);
    
    //Write the image to a file. Supported extensions: png, bmp, tga, jpg, jpeg
    //Returns true on success.
    bool saveToFile(const string& filename);
    
    void clear();
    
    Vector2i getSize() const { return size; }
    const uint32_t* getPtr() const { return &pixels[0]; }
    uint32_t* getPtr() { return &pixels[0]; }
    
    void draw(Ray2i ray, uint32_t color);
    void draw(Rect2i rect, uint32_t color);
    void draw(Vector2i position, const Image& img);
    void drawFilled(Rect2i rect, uint32_t color);
    void drawCircle(Vector2i position, int radius, uint32_t color);
    void drawFilledCircle(Vector2i position, int radius, uint32_t color);
    void drawFloodFill(Vector2i position, uint32_t color);
    void drawFilledPolygon(const std::vector<Vector2i>& polygon, uint32_t color);
    
    Image subImage(Rect2i area) const;
private:
    Vector2i size;
    std::vector<uint32_t> pixels;
};

}//namespace sp


#endif//SP2_GRAPHICS_IMAGE_H
