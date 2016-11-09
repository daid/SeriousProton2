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

    std::vector<sp::MeshData::Vertex> vertices;
    vertices.emplace_back(sf::Vector3f(-scale.x, -scale.y, 0.0f), sp::Vector2f(0, 1));
    vertices.emplace_back(sf::Vector3f( scale.x, -scale.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f(-scale.x,  scale.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f(-scale.x,  scale.y, 0.0f), sp::Vector2f(0, 0));
    vertices.emplace_back(sf::Vector3f( scale.x, -scale.y, 0.0f), sp::Vector2f(1, 1));
    vertices.emplace_back(sf::Vector3f( scale.x,  scale.y, 0.0f), sp::Vector2f(1, 0));

    data.shader = sp::Shader::get("shader/basic.shader");
    data.type = sp::RenderData::Type::Normal;
    data.mesh = std::make_shared<sp::MeshData>(vertices);
    data.texture = texture;

    return data;
}

};//!namespace sp
