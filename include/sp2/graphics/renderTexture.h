#ifndef SP2_GRAPHICS_RENDER_TEXTURE_H
#define SP2_GRAPHICS_RENDER_TEXTURE_H

#include <sp2/graphics/texture.h>
#include <sp2/math/vector.h>


namespace sp {

//NOTE: Untested
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
    RenderTexture(sp::string name, Vector2i size, bool double_buffered);
    virtual ~RenderTexture();
    
    //Get the texture for rendering to other targets.
    virtual void bind() override;
    
    Vector2i getSize() const;

    //Active rendering towards this texture, if we are double buffered, this flips the buffers.
    void activateRenderTarget();
private:
    void create();

    bool double_buffered;
    Vector2i size;
    
    bool dirty[2];
    bool flipped;
    bool auto_clear;
    
    unsigned int frame_buffer[2];
    unsigned int color_buffer[2];
    unsigned int depth_buffer[2];
    unsigned int stencil_buffer[2];
};

};//namespace sp

#endif//RENDER_TEXTURE_H
