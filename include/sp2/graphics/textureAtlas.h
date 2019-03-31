#ifndef SP2_GRAPHICS_TEXTURE_ATLAS_H
#define SP2_GRAPHICS_TEXTURE_ATLAS_H

#include <sp2/graphics/texture.h>

namespace sp {

class AtlasTexture : public Texture
{
public:
    AtlasTexture(Vector2i size);
    virtual ~AtlasTexture();
    
    virtual void bind() override;
    
    //Add an image to the atlas and return the area where the image is located in normalized coordinates.
    //Returns a negative size if the image cannot be added.
    Rect2f add(Image&& image, int margin=0);
private:
    void addArea(Rect2i area);

    unsigned int gl_handle;

    Vector2i texture_size;
    std::vector<Rect2i> available_areas;
    
    class ToAdd
    {
    public:
        Image image;
        Vector2i position;
    };
    std::vector<ToAdd> add_list;
};


};//namespace sp


#endif//SP2_GRAPHICS_TEXTURE_ATLAS_H
