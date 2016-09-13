#ifndef SP2_GRAPHICS_FONTMANAGER_H
#define SP2_GRAPHICS_FONTMANAGER_H

#include <sp2/io/directLoader.h>
#include <SFML/Graphics/Font.hpp>
#include <sp2/io/resourceProvider.h>

namespace sp {

class FontManager : public io::DirectLoader<sf::Font>
{
protected:
    virtual sf::Font* load(string name);
private:
    std::map<string, io::ResourceStreamPtr> resource_streams;
};

extern FontManager fontManager;

};//!namespace sp

#endif//SP2_GRAPHICS_FONTMANAGER_H
