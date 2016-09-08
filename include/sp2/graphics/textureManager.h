#ifndef SP2_GRAPHICS_TEXTUREMANAGER_H
#define SP2_GRAPHICS_TEXTUREMANAGER_H

#include <sp2/pointer.h>
#include <sp2/string.h>
#include <SFML/Graphics/Texture.hpp>
#include <map>

namespace sp {

class TextureManager
{
public:
    static sf::Texture* get(string name);
private:
    static sf::Texture* fallback_texture;
    static std::map<string, sf::Texture*> textures;
};

};//!namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H

