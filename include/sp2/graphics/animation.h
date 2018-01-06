#ifndef SP2_GRAPHICS_ANIMATION_H
#define SP2_GRAPHICS_ANIMATION_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/string.h>

namespace sp {

class Animation : sf::NonCopyable
{
public:
    virtual void play(string key, float speed=1.0) = 0;
    virtual void setFlags(int flags) = 0;
protected:
    virtual void update(float delta, RenderData& render_data) = 0;

    friend class Scene;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SPRITE_MANAGER_H
