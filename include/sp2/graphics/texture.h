#ifndef SP2_GRAPHICS_TEXTURE_H
#define SP2_GRAPHICS_TEXTURE_H

#include <sp2/string.h>
#include <sp2/nonCopyable.h>
#include <SFML/Graphics/Texture.hpp>
#include <thread>
#include <mutex>

namespace sp {

class Texture : NonCopyable
{
public:
    enum class Type
    {
        Static,
        Dynamic
    };
    const sf::Texture* get();
    int getRevision() { return revision; }
protected:
    Texture(sf::Texture& texture, Type type, string name)
    : texture(texture), type(type), name(name), revision(0) {}

    std::mutex mutex;
    sf::Texture& texture;
    
    std::shared_ptr<sf::Image> image;
    Type type;
    string name;
    int revision;
};

};//!namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H
