#ifndef SP2_GRAPHICS_SPRITE_MANAGER_H
#define SP2_GRAPHICS_SPRITE_MANAGER_H

#include <sp2/graphics/scene/renderdata.h>

namespace sp {

class SpriteManager
{
public:
    static const RenderData& get(string name);

    static RenderData& create(string name, string texture, float scale=1.0);
private:
    static std::map<string, RenderData> sprites;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SPRITE_MANAGER_H
