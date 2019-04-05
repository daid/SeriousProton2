#include <sp2/graphics/fontManager.h>

namespace sp {

FontManager font_manager;

Font* FontManager::load(string name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    if (!stream)
    {
        LOG(Warning, "Failed to find font file:", name);
        return nullptr;
    }
    if (name.endswith(".txt"))
        return new BitmapFont(name, stream);
    return new FreetypeFont(name, stream);
}

};//namespace sp
