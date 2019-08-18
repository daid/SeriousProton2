#ifndef SP2_GRAPHICS_SPRITE_ANIMATION_H
#define SP2_GRAPHICS_SPRITE_ANIMATION_H

#include <sp2/graphics/scene/renderdata.h>
#include <sp2/graphics/animation.h>
#include <sp2/math/vector.h>
#include <sp2/string.h>

namespace sp {

class AtlasManager;

/**
    The SpriteAnimation class can be attached to Node to provide flipbook like sprite animations.
    The SpriteAnimation class provides a set of animation as well as horizontal flipping.
    
    SpriteAnimation are loaded from a KeyValueTree text files, and each node with an ID provides an animation.
    Sprites are defined 
    
    The following keys are used from the tree node, including from parent nodes:
    - texture: [String] Texture to use for the animation. Full path.
    - texture_size: [Vector2i] Size in pixels of the texture.
    - loop: [Boolean] True if the animation should loop, else it plays once.
    - frames: [List of integers], indicates the sequence of frames to play. "0, 1, 0, 2" defines a 4 frame animation where 1 sprite is reused.
    - frame_count: [Integer] If "frames" is not set, this sets frames to a sequence from 0 to the given number (excluding the given number)
    - flip: [List of strings], Allows to set a H or V or HV flip per frame.
    - line_length: [Integer], Amount of sprites in sequence on a single row.
    - delay: [List of floats], Delay of each frame, should have the same length as "frames".
    - position: [Vector2f] Position of the first sprite in the spritesheet, in pixels.
    - frame_size: [Vector2f] Size of the sprites, in pixels.
    - offset: [Vector2f] Offset of the sprites compared to point of origin. 0,0 centers the sprite.
    - size: [Vector2f] Size of the final sprite in the engine. Generally a factor of the frame_size.
    - margin: [Vector2f] Margin between each sprite in the sprite sheet in pixels.
    
    Minimal required are:
    texture, texture_size, frames, position, size, frame_size

    AtlasManager:
        An AtlasManager can be set with setAtlasManager, this will place all sprites in a global atlas
        managed by the atlasmanager. This means less texture swaps need to be done, at the expensive
        of a bit longer loading times. It also has the effect of "texture_size" no longer being required.
*/
class SpriteAnimation : public Animation
{
public:
    virtual void play(string key, float speed=1.0) override;
    virtual void setFlags(int flags) override;
    virtual int getFlags() override;
    virtual bool finished() override;
protected:
    virtual void prepare(RenderData& render_data) override;
    virtual void update(float delta, RenderData& render_data) override;

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
                Texture* texture;
                std::shared_ptr<MeshData> normal_mesh;
                std::shared_ptr<MeshData> mirrored_mesh;
                float delay;
            };
            
            std::vector<Frame> frames;
            bool loop;
        };
        
        std::map<string, Animation> animations;
#ifdef DEBUG
        string resource_name;
        std::chrono::system_clock::time_point resource_update_time;
        int revision;
#endif

        void load(string resource_name);
    };

    SpriteAnimation(const Data& data);

    const Data& data;
    const Data::Animation* animation;
    float time_delta;
    float speed;
    bool playing;
    unsigned int keyframe;
    bool flip;
#ifdef DEBUG
    int revision;
#endif
public:
    static std::unique_ptr<Animation> load(string resource_name);
    static constexpr int FlipFlag = 0x0001;

    //Set a global atlas manager, newly loaded animations will be added to the atlas manager,
    static void setAtlasManager(AtlasManager* atlas_manager);
private:
    static std::map<string, Data*> cache;
    static AtlasManager* atlas_manager;
};

};//namespace sp

#endif//SP2_GRAPHICS_SPRITE_ANIMATION_H
