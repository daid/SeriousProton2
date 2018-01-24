#include <sp2/graphics/spriteAnimation.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/io/keyValueTreeLoader.h>
#include <sp2/logging.h>

namespace sp {

SpriteAnimation::SpriteAnimation(const Data& data)
: data(data), animation(nullptr)
{
    flip = false;
}

void SpriteAnimation::play(string key, float speed)
{
    auto it = data.animations.find(key);
    if (it == data.animations.end())
    {
        animation = nullptr;
        return;
    }
    this->speed = speed;
    playing = true;
    if (animation == &it->second)
        return;
    
    animation = &it->second;
    keyframe = 0;
    time_delta = 0;
}

void SpriteAnimation::setFlags(int flags)
{
    flip = flags & FlipFlag;
}

void SpriteAnimation::update(float delta, RenderData& render_data)
{
    if (!animation)
    {
        render_data.type = RenderData::Type::None;
        return;
    }
    if (playing)
    {
        time_delta += delta * speed;
        if (time_delta >= animation->frames[keyframe].delay)
        {
            time_delta -= animation->frames[keyframe].delay;
            keyframe = keyframe + 1;
            if (keyframe == animation->frames.size())
            {
                if (animation->loop)
                {
                    keyframe = 0;
                }
                else
                {
                    keyframe--;
                    playing = false;
                }
            }
        }
    }

    const Data::Animation::Frame& frame = animation->frames[keyframe];

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = animation->texture;
    render_data.type = RenderData::Type::Normal;
    if (flip)
        render_data.mesh = frame.mirrored_mesh;
    else
        render_data.mesh = frame.normal_mesh;
}

std::map<string, SpriteAnimation::Data> SpriteAnimation::cache;

std::unique_ptr<Animation> SpriteAnimation::load(string resource_name)
{
    Data& result = cache[resource_name];
    if (!result.animations.empty())
        return std::unique_ptr<Animation>(new SpriteAnimation(result));

    P<KeyValueTree> tree = io::KeyValueTreeLoader::load(resource_name);
    if (!tree)
        return std::unique_ptr<Animation>(new SpriteAnimation(result));
    int total_frames = 0;
    for(auto& it : tree->getFlattenNodesByIds())
    {
        Data::Animation& animation = result.animations[it.first];
        std::map<string, string>& data = it.second;
        
        animation.texture = textureManager.get(data["texture"]);
        animation.loop = stringutil::convert::toBool(data["loop"]);
        
        std::vector<int> frames = stringutil::convert::toIntArray(data["frames"]);
        if (frames.size() == 1)
        {
            int frame_count = stringutil::convert::toInt(data["frame_count"]);
            if (frame_count < 1)
                frame_count = 1;
            frames.clear();
            for(int n=0; n<frame_count; n++)
                frames.push_back(n);
        }
        int line_length = stringutil::convert::toInt(data["line_length"]);
        if (line_length <= 0)
            line_length = frames.size();
        std::vector<float> delays = stringutil::convert::toFloatArray(data["delay"]);
        sf::Vector2f texture_size = stringutil::convert::toVector2f(data["texture_size"]);
        sf::Vector2f position = stringutil::convert::toVector2f(data["position"]);
        sf::Vector2f offset = stringutil::convert::toVector2f(data["offset"]);
        sf::Vector2f frame_size = stringutil::convert::toVector2f(data["frame_size"]);
        sf::Vector2f size = stringutil::convert::toVector2f(data["size"]) / 2.0f;
        sf::Vector2f margin = stringutil::convert::toVector2f(data["margin"]);
        
        offset.x = offset.x / frame_size.x * size.x * 2.0;
        offset.y = offset.y / frame_size.y * size.y * 2.0;
        
        sf::Vector3f p0 = sf::Vector3f(-size.x + offset.x, -size.y + offset.y, 0.0f);
        sf::Vector3f p1 = sf::Vector3f( size.x + offset.x, -size.y + offset.y, 0.0f);
        sf::Vector3f p2 = sf::Vector3f(-size.x + offset.x,  size.y + offset.y, 0.0f);
        sf::Vector3f p3 = sf::Vector3f( size.x + offset.x,  size.y + offset.y, 0.0f);
        
        total_frames += frames.size();
        for(unsigned int n=0; n<frames.size(); n++)
        {
            int x = frames[n] % line_length;
            int y = frames[n] / line_length;
            
            float u0 = position.x + (frame_size.x + margin.x) * x;
            float u1 = u0 + frame_size.x;
            float v0 = position.y + (frame_size.y + margin.y) * y;
            float v1 = v0 + frame_size.y;
            
            u0 /= texture_size.x;
            u1 /= texture_size.x;
            v0 /= texture_size.y;
            v1 /= texture_size.y;

            animation.frames.emplace_back();
            Data::Animation::Frame& frame = animation.frames.back();
            
            sp::MeshData::Vertices vertices;
            vertices.emplace_back(p0, sp::Vector2f(u0, v1));
            vertices.emplace_back(p1, sp::Vector2f(u1, v1));
            vertices.emplace_back(p2, sp::Vector2f(u0, v0));
            vertices.emplace_back(p2, sp::Vector2f(u0, v0));
            vertices.emplace_back(p1, sp::Vector2f(u1, v1));
            vertices.emplace_back(p3, sp::Vector2f(u1, v0));
            frame.normal_mesh = MeshData::create(std::move(vertices));
            vertices.clear();
            vertices.emplace_back(p0, sp::Vector2f(u1, v1));
            vertices.emplace_back(p1, sp::Vector2f(u0, v1));
            vertices.emplace_back(p2, sp::Vector2f(u1, v0));
            vertices.emplace_back(p2, sp::Vector2f(u1, v0));
            vertices.emplace_back(p1, sp::Vector2f(u0, v1));
            vertices.emplace_back(p3, sp::Vector2f(u0, v0));
            frame.mirrored_mesh = MeshData::create(std::move(vertices));
            vertices.clear();
            
            frame.delay = delays[n % delays.size()];
        }
    }
    LOG(Info, "Got", result.animations.size(), "animations, with a total of", total_frames, "frames");
    
    tree.destroy();
    return std::unique_ptr<Animation>(new SpriteAnimation(result));
}

};//!namespace sp
