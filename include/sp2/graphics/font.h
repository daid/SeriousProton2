#ifndef SP2_GRAPHICS_FONT_H
#define SP2_GRAPHICS_FONT_H

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <sp2/alignment.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/texture.h>
#include <unordered_map>
#include <unordered_set>

namespace sp {

class Font : sf::NonCopyable
{
public:
    /** Create a meshData for the given string.
        This potential updates the texture, which can invalidate all previous created MeshData objects for this pixel size.
        Compare the texture revision to the revision that it initially had to check if the data is still valid or needs to be re-generated.
     */
    std::shared_ptr<MeshData> createString(string s, int pixel_size, float text_size, Vector2d area_size, Alignment alignment);
    Texture* getTexture(int pixel_size);
private:
    Font(string name, io::ResourceStreamPtr stream);

    string name;
    
    std::shared_ptr<sf::Font> font;
    //We need to keep the resource stream open, as the sf::Font instance keeps it open as well.
    //So we store the reference here.
    io::ResourceStreamPtr font_resource_stream;
    
    //Keep a cache of wrapped sf::Texture objects per font size
    std::unordered_map<int, Texture*> texture_cache;
    //Keep track of glyphs that are loaded in the texture already.
    //As soon as we load a new glyph, the texture becomes invalid and needs to be updated.
    std::unordered_map<int, std::unordered_set<int>> loaded_glyphs;
    
    Texture* bitmap_font;
    Vector2d bitmap_glyph_size;
    std::unordered_map<int, Vector2d> bitmap_glyphs;
    
    friend class FontManager2;
};

};//!namespace sp

#endif//SP2_GRAPHICS_FONT_H
