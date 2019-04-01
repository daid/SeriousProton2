#ifndef SP2_GRAPHICS_TEXTURE_ATLAS_H
#define SP2_GRAPHICS_TEXTURE_ATLAS_H

#include <sp2/graphics/texture.h>

namespace sp {

/**
    An AtlasTexture is a texture that contains multiple images layed out inside the same texture unit.
    The advantage of this is that there are less texture state changes during rendering, which is an inefficient
    action.
 */
class AtlasTexture : public Texture
{
public:
    AtlasTexture(string name, Vector2i size);
    virtual ~AtlasTexture();
    
    virtual void bind() override;

    //Only check if we can add this image, while this does the same work as add(), it does not claim ownership of the image
    //And thus the image can be placed somewhere else if this check fails.
    bool canAdd(const Image& image, int margin=0);
    
    //Add an image to the atlas and return the area where the image is located in normalized coordinates.
    //Returns a negative size if the image cannot be added.
    Rect2f add(Image&& image, int margin=0);
    
    //Return between 0.0 and 1.0 to indicate how much area of this texture is already used.
    // Where 0.0 is fully empty and 1.0 is fully used (never really happens due to overhead)
    float usageRate();
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

/**
    The AtlasManager is designed to allow virtually unlimited images to be loaded and placed into one or more atlas textures.
    This has the performance increasement of AtlasTextures but the ease of use of basic textureManager textures.
    The disadvantage of textures in the AtlasTexture is that they do not wrap around.
    
    Loaded images are cached and retrieving the same image is relatively quick and efficient.
 */
class AtlasManager
{
public:
    AtlasManager(Vector2i texture_size, int default_margin = 1);
    ~AtlasManager();
    
    struct Result
    {
        Texture* texture;
        Rect2f rect;
    };
    
    Result get(string resource_name);
private:
    Vector2i texture_size;
    int default_margin;

    std::vector<AtlasTexture*> textures;
    std::unordered_map<string, Result> cached_items;
};

};//namespace sp

#endif//SP2_GRAPHICS_TEXTURE_ATLAS_H
