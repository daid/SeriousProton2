#include <sp2/collision/2d/chains.h>
#include <sp2/scene/tilemap.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/assert.h>
#include <sp2/logging.h>

namespace sp {

Tilemap::Tilemap(P<Node> parent, const string& texture, float tile_size, int texture_tile_count)
: Tilemap(parent, texture, tile_size, tile_size, texture_tile_count, texture_tile_count)
{
}

Tilemap::Tilemap(P<Node> parent, const string& texture, float tile_width, float tile_height, int texture_tile_count_x, int texture_tile_count_y)
: Node(parent), tiles({.index = -1, .z_offset = 0.0, .normal = {0,0,1}, .collision = Collision::Open})
{
    sp2assert(tile_width > 0, "Tile width must be larger then zero");
    sp2assert(tile_height > 0, "Tile height must be larger then zero");
    sp2assert(texture_tile_count_x > 0, "Texture tile count must be larger then zero");
    sp2assert(texture_tile_count_y > 0, "Texture tile count must be larger then zero");
    
    this->tile_width = tile_width;
    this->tile_height = tile_height;

    this->texture_tile_count = Vector2i(texture_tile_count_x, texture_tile_count_y);

    render_data.shader = Shader::get("internal:basic.shader");
    if (texture != "")
        render_data.texture = texture_manager.get(texture);
    render_data.type = RenderData::Type::Normal;
    render_data.order = -1;
    
    dirty = false;
}

void Tilemap::setTilemapSpacingMargin(float spacing, float margin)
{
    texture_spacing.x = spacing / (float(texture_tile_count.x) + spacing * float(texture_tile_count.x - 1));
    texture_spacing.y = spacing / (float(texture_tile_count.y) + spacing * float(texture_tile_count.y - 1));
    texture_margin = Vector2f(margin, margin);
    
    dirty = true;
}

void Tilemap::setTile(sp::Vector2i position, int index, Collision collision)
{
    setTile(position, index, 0.0, {0, 0, 1}, collision);
}

void Tilemap::setTile(sp::Vector2i position, int index, double z_offset, sp::Vector3f normal, Collision collision)
{
    tiles.set(position, {.index = index, .z_offset = z_offset, .normal = normal, .collision = collision});
    dirty = true;
}

int Tilemap::getTileIndex(sp::Vector2i position)
{
    return tiles.get(position).index;
}

Tilemap::Collision Tilemap::getTileCollision(Vector2i position)
{
    return tiles.get(position).collision;
}

void Tilemap::onFixedUpdate()
{
    if (!dirty)
        return;
    dirty = false;
    updateMesh();
    updateCollision();
}

void Tilemap::updateMesh()
{
    Vector2f uv_step_size = Vector2f(
        (1.0 - texture_margin.x * 2.0f + texture_spacing.x) / float(texture_tile_count.x),
        (1.0 - texture_margin.y * 2.0f + texture_spacing.y) / float(texture_tile_count.y));
    Vector2f uv_size = uv_step_size - texture_spacing;

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    for(auto it : tiles) {
        const auto& tile = it.data;
        if (tile.index < 0)
            continue;
        int tile_index = tile.index & ~(flip_horizontal | flip_vertical | flip_diagonal);
        float px = it.position.x * tile_width;
        float py = it.position.y * tile_height;
        int u = tile_index % texture_tile_count.x;
        int v = tile_index / texture_tile_count.x;
        float u0 = texture_margin.x + u * uv_step_size.x;
        float v0 = texture_margin.y + v * uv_step_size.y;
        float u1 = u0 + uv_size.x;
        float v1 = v0 + uv_size.y;

        if (tile.index & flip_horizontal)
            std::swap(u0, u1);
        if (tile.index & flip_vertical)
            std::swap(v0, v1);

        indices.emplace_back(vertices.size() + 0);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 2);
        indices.emplace_back(vertices.size() + 1);
        indices.emplace_back(vertices.size() + 3);
        
        if (tile.index & flip_diagonal)
        {
            std::swap(u0, u1);
            std::swap(v0, v1);
            vertices.emplace_back(Vector3f(px, py, tile.z_offset), tile.normal, Vector2f(u1, v0));
            vertices.emplace_back(Vector3f(px + tile_width, py, tile.z_offset), tile.normal, Vector2f(u1, v1));
            vertices.emplace_back(Vector3f(px, py + tile_height, tile.z_offset), tile.normal, Vector2f(u0, v0));
            vertices.emplace_back(Vector3f(px + tile_width, py + tile_height, tile.z_offset), tile.normal, Vector2f(u0, v1));
        }
        else
        {
            vertices.emplace_back(Vector3f(px, py, tile.z_offset), tile.normal, Vector2f(u0, v1));
            vertices.emplace_back(Vector3f(px + tile_width, py, tile.z_offset), tile.normal, Vector2f(u1, v1));
            vertices.emplace_back(Vector3f(px, py + tile_height, tile.z_offset), tile.normal, Vector2f(u0, v0));
            vertices.emplace_back(Vector3f(px + tile_width, py + tile_height, tile.z_offset), tile.normal, Vector2f(u1, v0));
        }
    }
    
    if (!render_data.mesh)
        render_data.mesh = MeshData::create(std::move(vertices), std::move(indices));
    else
        render_data.mesh->update(std::move(vertices), std::move(indices));
}

class TilemapCollisionBuilder
{
public:
    sp::collision::Chains2D result;
    
    TilemapCollisionBuilder(Tilemap& tilemap)
    : solid(false), up(false), down(false), left(false), right(false), tilemap(tilemap)
    {
        result.type = collision::Shape::Type::Static;

        for(auto it : tilemap.tiles)
        {
            if (it.data.collision == Tilemap::Collision::Solid)
                solid.set(it.position, true);
        }
        for(auto it : solid)
        {
            if (!it.data) continue;
            if (!solid.get(it.position + Vector2i{0, 1})) up.set(it.position, true);
            if (!solid.get(it.position + Vector2i{0,-1})) down.set(it.position, true);
            if (!solid.get(it.position + Vector2i{1, 0})) right.set(it.position, true);
            if (!solid.get(it.position + Vector2i{-1,0})) left.set(it.position, true);
        }
        for(auto it : up) {
            if (!it.data) continue;
            collision::Chains2D::Path path;
            buildPathUp(path, it.position);
            result.loops.emplace_back(std::move(path));
        }
        for(auto it : tilemap.tiles)
        {
            if (it.data.collision == Tilemap::Collision::Platform && tilemap.tiles.get(it.position - Vector2i{1, 0}).collision != Tilemap::Collision::Platform)
            {
                int x0 = it.position.x;
                while(tilemap.tiles.get(it.position).collision == Tilemap::Collision::Platform)
                {
                    it.position.x++;
                }
                if (x0 != it.position.x)
                {
                    collision::Chains2D::Path path;
                    path.emplace_back(x0, it.position.y+1);
                    path.emplace_back(it.position.x, it.position.y+1);
                    result.chains.emplace_back(std::move(path));
                }
            }
        }
    }
private:
    InfiniGrid<bool> solid;
    InfiniGrid<bool> up;
    InfiniGrid<bool> down;
    InfiniGrid<bool> left;
    InfiniGrid<bool> right;
    Tilemap& tilemap;

    void buildPathUp(collision::Chains2D::Path& path, Vector2i position)
    {
        for(;up.get(position); position.x++)
            up.set(position, false);
        path.emplace_back(position.x * tilemap.tile_width, (position.y + 1) * tilemap.tile_height);
        if (right.get({position.x - 1, position.y}))
        {
            buildPathRight(path, {position.x - 1, position.y});
            return;
        }
        if (left.get({position.x, position.y + 1}))
        {
            buildPathLeft(path, {position.x, position.y + 1});
            return;
        }
    }

    void buildPathLeft(collision::Chains2D::Path& path, Vector2i position)
    {
        for(; left.get(position); position.y++)
            left.set(position, false);
        path.emplace_back(position.x * tilemap.tile_width, position.y * tilemap.tile_height);
        if (up.get({position.x, position.y - 1}))
        {
            buildPathUp(path, {position.x, position.y - 1});
            return;
        }
        if (down.get({position.x - 1, position.y}))
        {
            buildPathDown(path, {position.x - 1, position.y});
            return;
        }
    }

    void buildPathRight(collision::Chains2D::Path& path, Vector2i position)
    {
        for(; right.get(position); position.y--)
            right.set(position, false);
        path.emplace_back((position.x + 1) * tilemap.tile_width, (position.y + 1) * tilemap.tile_height);
        if (down.get({position.x, position.y + 1}))
        {
            buildPathDown(path, {position.x, position.y + 1});
            return;
        }
        if (up.get({position.x + 1, position.y}))
        {
            buildPathUp(path, {position.x + 1, position.y});
            return;
        }
    }

    void buildPathDown(collision::Chains2D::Path& path, Vector2i position)
    {
        for(; down.get(position); position.x--)
            down.set(position, false);
        path.emplace_back((position.x + 1) * tilemap.tile_width, position.y * tilemap.tile_height);
        if (right.get({position.x, position.y - 1}))
        {
            buildPathRight(path, {position.x, position.y - 1});
            return;
        }
        if (left.get({position.x + 1, position.y}))
        {
            buildPathLeft(path, {position.x + 1, position.y});
            return;
        }
    }
};

void Tilemap::updateCollision()
{
    TilemapCollisionBuilder builder(*this);
    if (!builder.result.chains.empty() || !builder.result.loops.empty())
        setCollisionShape(builder.result);
    else
        removeCollisionShape();
}

}//namespace sp
