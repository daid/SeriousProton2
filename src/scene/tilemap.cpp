#include <sp2/collision/2d/chains.h>
#include <sp2/scene/tilemap.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/assert.h>
#include <sp2/logging.h>

namespace sp {

Tilemap::Tilemap(P<Node> parent, string texture, float tile_size, int texture_tile_count)
: Tilemap(parent, texture, tile_size, tile_size, texture_tile_count, texture_tile_count)
{
}

Tilemap::Tilemap(P<Node> parent, string texture, float tile_width, float tile_height, int texture_tile_count_x, int texture_tile_count_y)
: Node(parent)
{
    sp2assert(tile_width > 0, "Tile width must be larger then zero");
    sp2assert(tile_height > 0, "Tile height must be larger then zero");
    sp2assert(texture_tile_count_x > 0, "Texture tile count must be larger then zero");
    sp2assert(texture_tile_count_y > 0, "Texture tile count must be larger then zero");
    
    this->tile_width = tile_width;
    this->tile_height = tile_height;

    this->texture_tile_count = Vector2i(texture_tile_count_x, texture_tile_count_y);

    render_data.shader = Shader::get("internal:basic.shader");
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

void Tilemap::setTile(int x, int y, int index, Collision collision)
{
    sp2assert(x >= 0, "Tile position must be equal or larger then zero");
    sp2assert(y >= 0, "Tile position must be equal or larger then zero");
    
    int width = x + 1;
    if (tiles.size())
        width = std::max(int(tiles[0].size()), width);
    if(int(tiles.size()) < y + 1)
    {
        int old_height = tiles.size();
        tiles.resize(y + 1);
        for(int n=old_height; n<int(tiles.size()); n++)
            tiles[n].resize(width);
    }
    if (int(tiles[0].size()) != width)
    {
        for(int n=0; n<int(tiles.size()); n++)
            tiles[n].resize(width);
    }
    
    tiles[y][x].index = index;
    tiles[y][x].collision = collision;
    dirty = true;
}

void Tilemap::setTileZOffset(int x, int y, double z_offset)
{
    sp2assert(x >= 0, "Tile position must be equal or larger then zero");
    sp2assert(y >= 0, "Tile position must be equal or larger then zero");
    
    int width = x + 1;
    if (tiles.size())
        width = std::max(int(tiles[0].size()), width);
    if(int(tiles.size()) < y + 1)
    {
        int old_height = tiles.size();
        tiles.resize(y + 1);
        for(int n=old_height; n<int(tiles.size()); n++)
            tiles[n].resize(width);
    }
    if (int(tiles[0].size()) != width)
    {
        for(int n=0; n<int(tiles.size()); n++)
            tiles[n].resize(width);
    }
    
    tiles[y][x].z_offset = z_offset;
    dirty = true;
}

int Tilemap::getTileIndex(int x, int y)
{
    if (y < 0 || y >= int(tiles.size()))
        return -1;
    if (x < 0 || x >= int(tiles[0].size()))
        return -1;
    return tiles[y][x].index;
}

Tilemap::Collision Tilemap::getTileCollision(int x, int y)
{
    if (y < 0 || y >= int(tiles.size()))
        return Collision::Open;
    if (x < 0 || x >= int(tiles[0].size()))
        return Collision::Open;
    return tiles[y][x].collision;
}

Vector2i Tilemap::getSize()
{
    if (tiles.size() < 1)
        return Vector2i(0, 0);
    return Vector2i(tiles[0].size(), tiles.size());
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
    for(unsigned int y=0; y<tiles.size(); y++)
    {
        for(unsigned int x=0; x<tiles[y].size(); x++)
        {
            const auto& tile = tiles[y][x];
            if (tile.index < 0)
                continue;
            int tile_index = tile.index & ~(flip_horizontal | flip_vertical);
            float px = x * tile_width;
            float py = y * tile_height;
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
            
            vertices.emplace_back(Vector3f(px, py, tile.z_offset), Vector2f(u0, v1));
            vertices.emplace_back(Vector3f(px + tile_width, py, tile.z_offset), Vector2f(u1, v1));
            vertices.emplace_back(Vector3f(px, py + tile_height, tile.z_offset), Vector2f(u0, v0));
            vertices.emplace_back(Vector3f(px + tile_width, py + tile_height, tile.z_offset), Vector2f(u1, v0));
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
    : tilemap(tilemap)
    {
        result.type = collision::Shape::Type::Static;
    
        w = tilemap.tiles[0].size();
        h = tilemap.tiles.size();
        collision.resize(w);
        for(int x=0; x<w; x++)
            collision[x].resize(h);

        for(int y=0; y<h; y++)
        {
            for(int x=0; x<w; x++)
            {
                const auto& tile = tilemap.tiles[y][x];
                
                collision[x][y].solid = tile.collision == Tilemap::Collision::Solid;
            }
        }
        for(int y=0; y<h; y++)
        {
            for(int x=0; x<w; x++)
            {
                auto& col = collision[x][y];
                if (col.solid)
                {
                    col.up = (y + 1 == h || !collision[x][y+1].solid);
                    col.down = (y == 0 || !collision[x][y-1].solid);
                    col.right = (x + 1 == w || !collision[x+1][y].solid);
                    col.left = (x == 0 || !collision[x-1][y].solid);
                }
                else
                {
                    col.up = col.down = col.left = col.right = false;
                }
            }
        }
        for(int y=0; y<h; y++)
        {
            for(int x=0; x<w; x++)
            {
                auto& col = collision[x][y];
                if (col.up)
                {
                    collision::Chains2D::Path path;
                    buildPathUp(path, x, y);
                    result.loops.emplace_back(std::move(path));
                }
            }
        }
        for(int y=0; y<h; y++)
        {
            for(int x=0; x<w; x++)
            {
                int x0 = x;
                while(x<w && tilemap.tiles[y][x].collision == Tilemap::Collision::Platform)
                {
                    x++;
                }
                if (x0 != x)
                {
                    collision::Chains2D::Path path;
                    path.emplace_back(x0, y+1);
                    path.emplace_back(x, y+1);
                    result.chains.emplace_back(std::move(path));
                }
            }
        }
    }
private:
    class Info
    {
    public:
        bool solid;
        bool up, down, left, right;
    };
    std::vector<std::vector<Info>> collision;
    Tilemap& tilemap;
    int w, h;

    void buildPathUp(collision::Chains2D::Path& path, int x, int y)
    {
        sp2assert(x >= 0 && y >= 0 && x < w && y < h, "Internal logic error");
        for(; x<w && collision[x][y].up; x++)
        {
            collision[x][y].up = false;
        }
        path.emplace_back(x * tilemap.tile_width, (y + 1) * tilemap.tile_height);
        if (x > 0 && collision[x - 1][y].right)
        {
            buildPathRight(path, x - 1, y);
            return;
        }
        if (x<w && y + 1 < h && collision[x][y + 1].left)
        {
            buildPathLeft(path, x, y + 1);
            return;
        }
    }

    void buildPathLeft(collision::Chains2D::Path& path, int x, int y)
    {
        sp2assert(x >= 0 && y >= 0 && x < w && y < h, "Internal logic error");
        for(; y<h && collision[x][y].left; y++)
        {
            collision[x][y].left = false;
        }
        path.emplace_back(x * tilemap.tile_width, y * tilemap.tile_height);
        if (y > 0 && collision[x][y - 1].up)
        {
            buildPathUp(path, x, y - 1);
            return;
        }
        if (x > 0 && y < h && collision[x - 1][y].down)
        {
            buildPathDown(path, x - 1, y);
            return;
        }
    }

    void buildPathRight(collision::Chains2D::Path& path, int x, int y)
    {
        sp2assert(x >= 0 && y >= 0 && x < w && y < h, "Internal logic error");
        for(; y>=0 && collision[x][y].right; y--)
        {
            collision[x][y].right = false;
        }
        path.emplace_back((x + 1) * tilemap.tile_width, (y + 1) * tilemap.tile_height);
        if (y + 1 < h && collision[x][y + 1].down)
        {
            buildPathDown(path, x, y + 1);
            return;
        }
        if (x + 1 < w && y >= 0 && collision[x + 1][y].up)
        {
            buildPathUp(path, x + 1, y);
            return;
        }
    }

    void buildPathDown(collision::Chains2D::Path& path, int x, int y)
    {
        sp2assert(x >= 0 && y >= 0 && x < w && y < h, "Internal logic error");
        for(; x>=0 && collision[x][y].down; x--)
        {
            collision[x][y].down = false;
        }
        path.emplace_back((x + 1) * tilemap.tile_width, y * tilemap.tile_height);
        if (x >= 0 && y > 0 && collision[x][y - 1].right)
        {
            buildPathRight(path, x, y - 1);
            return;
        }
        if (x + 1 < w && collision[x + 1][y].left)
        {
            buildPathLeft(path, x + 1, y);
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

};//namespace sp
