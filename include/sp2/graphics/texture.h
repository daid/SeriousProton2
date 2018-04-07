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
    virtual const sf::Texture* get() = 0;
    int getRevision() { return revision; }
protected:
    Texture(Type type, string name)
    : type(type), name(name), revision(0) {}

    Type type;
    string name;
    int revision;
};

};//!namespace sp

#endif//SP2_GRAPHICS_TEXTUREMANAGER_H
