#ifndef SP2_GRAPHICS_IMAGE_H
#define SP2_GRAPHICS_IMAGE_H

#include <sp2/string.h>
#include <sp2/math/vector2.h>
#include <sp2/io/resourceProvider.h>


namespace sp {

class Image
{
public:
    Image();
    Image(Image&& other);
    Image(Vector2i size);
    
    void operator=(Image&& other);
    
    void update(Vector2i size, const uint32_t* ptr);
    bool loadFromStream(io::ResourceStreamPtr stream);
    
    //Write the image to a file. Supported extensions: png, bmp, tga, jpg, jpeg
    //Returns true on success.
    bool saveToFile(sp::string filename);
    
    void clear();
    
    Vector2i getSize() const { return size; }
    const uint32_t* getPtr() const { return pixels.data(); }
private:
    Vector2i size;
    std::vector<uint32_t> pixels;
};

};//namespace sp


#endif//SP2_GRAPHICS_SHADER_H
