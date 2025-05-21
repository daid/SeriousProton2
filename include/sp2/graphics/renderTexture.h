#ifndef SP2_GRAPHICS_RENDER_TEXTURE_H
#define SP2_GRAPHICS_RENDER_TEXTURE_H

#include <sp2/graphics/texture.h>
#include <sp2/math/vector.h>


namespace sp {

class RenderTexture : public Texture
{
public:
    /**
        Create a RenderTexture.
        This can be used as both a texture, as well as a target to render on.

        @param name: Name of this render target, for debugging.
        @param size: Size of the render target in pixels.
        @param double_buffered: Set this to true if rendering is required while reading from this texture.
            This will effectively create 2 textures, one for reading and writting, and flips the goals around each frame.
     */
    RenderTexture(const string& name, Vector2i size, bool double_buffered);
    RenderTexture(const string& name, Vector2i size, int texture_count, bool double_buffered);
    virtual ~RenderTexture();

    //Get the texture for rendering to other targets.
    virtual void bind() override;

    void setSize(Vector2i size);
    Vector2i getSize() const;

    Texture* getTexture(int index);

    //Active rendering towards this texture, if we are double buffered, this flips the buffers.
    void activateRenderTarget();
private:
    class ColorTexture : public Texture
    {
    public:
        ColorTexture(const string& name);
        virtual ~ColorTexture();

        virtual void bind() override;

        unsigned int handle = 0;
    };
    void create();

    bool double_buffered;
    Vector2i size;
    int texture_count=1;

    bool dirty[2];
    bool flipped;
    bool auto_clear;
    bool create_buffers = true;

    unsigned int frame_buffer[2];
    std::vector<ColorTexture*> color_buffer[2];
    unsigned int depth_buffer[2];
};

}//namespace sp

#endif//RENDER_TEXTURE_H
