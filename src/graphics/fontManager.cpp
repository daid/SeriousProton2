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
        if (!font->loadFromStream(*stream))
            LOG(Warning, "Failed to load font:", name);
    }
    return font;
}

FontManager2 font_manager;

Font* FontManager2::load(string name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    return new Font(name, stream);
}

};//!namespace sp
