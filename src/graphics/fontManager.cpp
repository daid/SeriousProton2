#include <sp2/graphics/fontManager.h>

namespace sp {

FontManager font_manager;

Font* FontManager::load(string name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    return new Font(name, stream);
}

};//!namespace sp
