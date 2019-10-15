#ifndef SP2_GRAPHICS_ANIMATION_H
#define SP2_GRAPHICS_ANIMATION_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/string.h>
#include <sp2/nonCopyable.h>

namespace sp {

class Animation : NonCopyable
{
public:
    virtual ~Animation() {}

    virtual void play(const string& key, float speed=1.0) = 0;
    virtual void setFlags(int flags) = 0;
    virtual int getFlags() = 0;

    virtual bool finished() = 0;

    virtual void prepare(RenderData& render_data) = 0;
    virtual void update(float delta, RenderData& render_data) = 0;
};

}//namespace sp

#endif//SP2_GRAPHICS_SPRITE_MANAGER_H
