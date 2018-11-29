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
    Image(Image&& other);
    Image(Vector2i size);
    Image(Vector2i size, uint32_t color);
    
    void operator=(Image&& other);
    
    void update(Vector2i size, const uint32_t* ptr);
    bool loadFromStream(io::ResourceStreamPtr stream);
    
    //Write the image to a file. Supported extensions: png, bmp, tga, jpg, jpeg
    //Returns true on success.
    bool saveToFile(sp::string filename);
    
    void clear();
    
    Vector2i getSize() const { return size; }
    const uint32_t* getPtr() const { return pixels.data(); }
    
    void draw(Ray2i ray, uint32_t color);
    void draw(Rect2i rect, uint32_t color);
    void drawFilled(Rect2i rect, uint32_t color);
    void drawCircle(Vector2i position, int radius, uint32_t color);
    void drawFilledCircle(Vector2i position, int radius, uint32_t color);
    void drawFloodFill(Vector2i position, uint32_t color);
    void drawFilledPolygon(const std::vector<Vector2i>& polygon, uint32_t color);
private:
    Vector2i size;
    std::vector<uint32_t> pixels;
};

};//namespace sp


#endif//SP2_GRAPHICS_SHADER_H
