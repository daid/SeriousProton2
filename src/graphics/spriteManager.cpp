#include <sp2/graphics/spriteManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/logging.h>

namespace sp {

std::map<string, RenderData> SpriteManager::sprites;
static RenderData no_sprite;

const RenderData& SpriteManager::get(string name)
{
    auto it = sprites.find(name);
    if (it == sprites.end())
    {
        LOG(Error, "Failed to find sprite:", name);
        return no_sprite;
    }
    return it->second;
}

RenderData& SpriteManager::create(string name, string texture, float scale)
{
    return create(name, texture, sp::Vector2f(scale, scale));
}

RenderData& SpriteManager::create(string name, string texture, sp::Vector2f scale)
{
    RenderData& data = sprites[name];

    data.shader = sp::Shader::get("shader/basic.shader");
    data.type = sp::RenderData::Type::Normal;
    data.mesh = sp::MeshData::createQuad(scale);
    data.texture = texture;

    return data;
}

};//!namespace sp
