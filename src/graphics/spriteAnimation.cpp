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
#ifdef DEBUG
    revision = data.revision;
#endif
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

int SpriteAnimation::getFlags()
{
    if (flip)
        return FlipFlag;
    return 0;
}

void SpriteAnimation::update(float delta, RenderData& render_data)
{
#ifdef DEBUG
    if (io::ResourceProvider::getModifyTime(data.resource_name) != data.resource_update_time)
    {
        string animation_name;
        for(auto it : data.animations)
            if (&it.second == animation)
                animation_name = it.first;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LOG(Info, "Reloading sprite animation:", data.resource_name);
        animation = nullptr;
        (const_cast<Data*>(&data))->load(data.resource_name);
        
        
        play(animation_name, speed);
    }
    if (data.revision != revision)
    {
        animation = nullptr;
        revision = data.revision;
    }
#endif
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

std::map<string, SpriteAnimation::Data*> SpriteAnimation::cache;

std::unique_ptr<Animation> SpriteAnimation::load(string resource_name)
{
    auto it = cache.find(resource_name);
    if (it != cache.end())
        return std::unique_ptr<Animation>(new SpriteAnimation(*it->second));
    Data* result = new Data();
#ifdef DEBUG
    result->revision = -1;
#endif
    result->load(resource_name);
    cache[resource_name] = result;
    return std::unique_ptr<Animation>(new SpriteAnimation(*result));
}

void SpriteAnimation::Data::load(string resource_name)
{
#ifdef DEBUG
    revision++;
#endif
    KeyValueTreePtr tree = io::KeyValueTreeLoader::load(resource_name);
    if (!tree)
        return;

    float u_offset = 0.0;
    float v_offset = 0.0;

    animations.clear();
    int total_frames = 0;
    for(auto& it : tree->getFlattenNodesByIds())
    {
        Data::Animation& animation = animations[it.first];
        std::map<string, string>& data = it.second;
        
        animation.texture = texture_manager.get(data["texture"]);
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
        std::vector<sp::string> flip = data["flip"].split(",");
        while(flip.size() < frames.size())
        {
            flip.push_back(flip[0]);
        }
        int line_length = stringutil::convert::toInt(data["line_length"]);
        if (line_length <= 0)
            line_length = frames.size();
        std::vector<float> delays = stringutil::convert::toFloatArray(data["delay"]);
        Vector2f texture_size = stringutil::convert::toVector2f(data["texture_size"]);
        Vector2f position = stringutil::convert::toVector2f(data["position"]);
        Vector2f offset = stringutil::convert::toVector2f(data["offset"]);
        Vector2f frame_size = stringutil::convert::toVector2f(data["frame_size"]);
        Vector2f size = stringutil::convert::toVector2f(data["size"]) / 2.0f;
        Vector2f margin = stringutil::convert::toVector2f(data["margin"]);
        
        offset.x = offset.x / frame_size.x * size.x * 2.0;
        offset.y = offset.y / frame_size.y * size.y * 2.0;
        
        Vector3f p0 = Vector3f(-size.x + offset.x, -size.y + offset.y, 0.0f);
        Vector3f p1 = Vector3f( size.x + offset.x, -size.y + offset.y, 0.0f);
        Vector3f p2 = Vector3f(-size.x + offset.x,  size.y + offset.y, 0.0f);
        Vector3f p3 = Vector3f( size.x + offset.x,  size.y + offset.y, 0.0f);
        
        total_frames += frames.size();
        for(unsigned int n=0; n<frames.size(); n++)
        {
            int x = frames[n] % line_length;
            int y = frames[n] / line_length;
            
            float u0 = position.x + (frame_size.x + margin.x) * x;
            float u1 = u0 + frame_size.x;
            float v0 = position.y + (frame_size.y + margin.y) * y;
            float v1 = v0 + frame_size.y;
            
            if (flip[n].strip().upper() == "H")
                std::swap(u0, u1);
            if (flip[n].strip().upper() == "V")
                std::swap(v0, v1);
            if (flip[n].strip().upper() == "HV")
            {
                std::swap(u0, u1);
                std::swap(v0, v1);
            }
            
            u0 /= texture_size.x;
            u1 /= texture_size.x;
            v0 /= texture_size.y;
            v1 /= texture_size.y;
            
            u0 += u_offset;
            u1 -= u_offset;
            v0 += v_offset;
            v1 -= v_offset;

            animation.frames.emplace_back();
            Data::Animation::Frame& frame = animation.frames.back();
            
            sp::MeshData::Vertices vertices;
            sp::MeshData::Indices indices{0,1,2, 2,1,3};
            vertices.emplace_back(p0, sp::Vector2f(u0, v1));
            vertices.emplace_back(p1, sp::Vector2f(u1, v1));
            vertices.emplace_back(p2, sp::Vector2f(u0, v0));
            vertices.emplace_back(p3, sp::Vector2f(u1, v0));
            frame.normal_mesh = MeshData::create(std::move(vertices), std::move(indices));
            vertices.clear();
            sp::MeshData::Indices indices2{0,1,2, 2,1,3};
            vertices.emplace_back(p0, sp::Vector2f(u1, v1));
            vertices.emplace_back(p1, sp::Vector2f(u0, v1));
            vertices.emplace_back(p2, sp::Vector2f(u1, v0));
            vertices.emplace_back(p3, sp::Vector2f(u0, v0));
            frame.mirrored_mesh = MeshData::create(std::move(vertices), std::move(indices2));
            vertices.clear();
            
            frame.delay = delays[n % delays.size()];
        }
    }
    LOG(Info, "Got", animations.size(), "animations, with a total of", total_frames, "frames");

#ifdef DEBUG
    this->resource_name = resource_name;
    this->resource_update_time = io::ResourceProvider::getModifyTime(resource_name);
#endif
}

};//namespace sp
