#ifndef RENDER_TEXTURE_H
#define RENDER_TEXTURE_H

#include <sp2/graphics/texture.h>
#include <sp2/math/vector.h>

#include <SFML/Graphics/RenderTexture.hpp>


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
    RenderTexture(sp::string name, Vector2i size, bool double_buffered);
    
    //Get the texture for rendering to other targets.
    virtual const sf::Texture* get() override;

    //Active rendering towards this texture.
    sf::RenderTarget& activateRenderTarget();
private:
    bool double_buffered;
    
    bool dirty[2];
    bool flipped;
    sf::RenderTexture render_texture[2];
};

};//namespace sp

#endif//RENDER_TEXTURE_H
