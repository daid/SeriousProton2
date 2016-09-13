#include <sp2/graphics/fontManager.h>

namespace sp {

FontManager fontManager;

sf::Font* FontManager::load(string name)
{
    sf::Font* font = new sf::Font();
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    if (stream)
    {
        resource_streams[name] = stream;
        LOG(Info, "Loading font:", name);
        font->loadFromStream(*stream);
    }
    return font;
}

};//!namespace sp
