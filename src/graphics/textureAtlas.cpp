#include <sp2/graphics/textureAtlas.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/opengl.h>

#include <algorithm>


namespace sp {

AtlasTexture::AtlasTexture(string name, Vector2i size)
: Texture(Texture::Type::Dynamic, name)
{
    texture_size = size;
    available_areas.emplace_back(0, 0, size.x, size.y);
    gl_handle = 0;
}

AtlasTexture::~AtlasTexture()
{
    if (gl_handle)
        glDeleteTextures(1, &gl_handle);
}

void AtlasTexture::bind()
{
    if (gl_handle == 0)
    {
        glGenTextures(1, &gl_handle);
        glBindTexture(GL_TEXTURE_2D, gl_handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_manager.isDefaultSmoothFiltering() ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_manager.isDefaultSmoothFiltering() ? GL_LINEAR : GL_NEAREST);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_size.x, texture_size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, gl_handle);
    }

    if (!add_list.empty())
    {
        for(auto& add_item : add_list)
            glTexSubImage2D(GL_TEXTURE_2D, 0, add_item.position.x, add_item.position.y, add_item.image.getSize().x, add_item.image.getSize().y, GL_RGBA, GL_UNSIGNED_BYTE, add_item.image.getPtr());
        revision++;
        
        add_list.clear();
    }
}

//Add an image to the atlas and return the area where the image is located in normalized coordinates.
//Returns a negative size if the image cannot be added.
Rect2f AtlasTexture::add(Image&& image, int margin)
{
    Vector2i size = image.getSize() + Vector2i(margin * 2, margin * 2);
    for(int n=int(available_areas.size()) - 1; n >= 0; n--)
    {
        if (available_areas[n].size.x >= size.x && available_areas[n].size.y >= size.y)
        {
            //Suitable area found, grab it, cut it, and put it in a stew.
            Rect2i full_area = available_areas[n];
            available_areas.erase(available_areas.begin() + n);
            
            if (full_area.size.x <= full_area.size.y)
            {
                //Split horizontal
                addArea(Rect2i(full_area.position.x + size.x, full_area.position.y, full_area.size.x - size.x, size.y));
                addArea(Rect2i(full_area.position.x, full_area.position.y + size.y, full_area.size.x, full_area.size.y - size.y));
            }
            else
            {
                //Split vertical
                addArea(Rect2i(full_area.position.x + size.x, full_area.position.y, full_area.size.x - size.x, full_area.size.y));
                addArea(Rect2i(full_area.position.x, full_area.position.y + size.y, size.x, full_area.size.y - size.y));
            }
            
            add_list.emplace_back();
            add_list.back().image = std::move(image);
            add_list.back().position.x = full_area.position.x + margin;
            add_list.back().position.y = full_area.position.y + margin;
            
            return Rect2f(
                float(full_area.position.x + margin) / float(texture_size.x), float(full_area.position.y + margin) / float(texture_size.y),
                float(size.x - margin * 2) / float(texture_size.x), float(size.y - margin * 2) / float(texture_size.y));
        }
    }
    return Rect2f(0, 0, -1, -1);
}

void AtlasTexture::addArea(Rect2i area)
{
    //Ignore really slim areas, they are never really used and use up a lot of room in the available_areas otherwise.
    if (area.size.x < 4 || area.size.y < 4)
        return;
    int area_volume = area.size.x * area.size.y;
    auto it = std::lower_bound(available_areas.begin(), available_areas.end(), area_volume, [](const Rect2i& r, int volume) -> bool
    {
        return r.size.x * r.size.y > volume;
    });
    available_areas.insert(it, area);
}

};//namespace sp
