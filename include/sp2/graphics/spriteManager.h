#ifndef SP2_GRAPHICS_SPRITE_MANAGER_H
#define SP2_GRAPHICS_SPRITE_MANAGER_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/graphics/animation.h>
#include <sp2/math/vector.h>
#include <sp2/string.h>

namespace sp {

class SpriteAnimation : public Animation
{
public:
    virtual void play(string key);
    virtual void setFlags(int flags);
protected:
    virtual void update(float delta, RenderData& render_data);

private:
    class Data
    {
    public:
        class Animation
        {
        public:
            class Frame
            {
            public:
                std::shared_ptr<MeshData> normal_mesh;
                std::shared_ptr<MeshData> mirrored_mesh;
                float delay;
            };
            
            string texture;
            std::vector<Frame> frames;
            bool loop;
        };
        
        std::map<string, Animation> animations;
    };

    SpriteAnimation(const Data& data);

    const Data& data;
    const Data::Animation* animation;
    float time_delta;
    unsigned int keyframe;
    bool flip;
public:
    static std::unique_ptr<Animation> load(string resource_name);
private:
    static std::map<string, Data> cache;
};

};//!namespace sp

#endif//SP2_GRAPHICS_SPRITE_MANAGER_H
