#include <sp2/collision/2d/chains.h>
#include <sp2/scene/tilemap.h>
#include <sp2/graphics/meshdata.h>
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

    this->texture_tile_count_x = texture_tile_count_x;
    this->texture_tile_count_y = texture_tile_count_y;

    render_data.shader = Shader::get("internal:basic.shader");
    render_data.texture = texture;
    render_data.type = RenderData::Type::Normal;
    
    dirty = false;
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

int Tilemap::getTileIndex(int x, int y)
{
    if (y < 0 || y >= tiles.size())
        return -1;
    if (x < 0 || x >= tiles[0].size())
        return -1;
    return tiles[y][x].index;
}

void Tilemap::onUpdate(float delta)
{
    if (!dirty)
        return;
    dirty = false;
    updateMesh();
    updateCollision();
}

void Tilemap::updateMesh()
{
    float fu = 1.0 / float(texture_tile_count_x);
    float fv = 1.0 / float(texture_tile_count_y);
    
    MeshData::Vertices vertices;
    for(unsigned int y=0; y<tiles.size(); y++)
    {
        for(unsigned int x=0; x<tiles[y].size(); x++)
        {
            const auto& tile = tiles[y][x];
            if (tile.index < 0)
                continue;
            float px = x * tile_width;
            float py = y * tile_height;
            int u = tile.index % texture_tile_count_x;
            int v = tile.index / texture_tile_count_x;
            
            vertices.emplace_back(sf::Vector3f(px, py, 0.0f), sp::Vector2f(u * fu, (v + 1) * fv));
            vertices.emplace_back(sf::Vector3f(px + tile_width, py, 0.0f), sp::Vector2f((u + 1) * fu, (v + 1) * fv));
            vertices.emplace_back(sf::Vector3f(px, py + tile_height, 0.0f), sp::Vector2f(u * fu, v * fv));
            vertices.emplace_back(sf::Vector3f(px, py + tile_height, 0.0f), sp::Vector2f(u * fu, v * fv));
            vertices.emplace_back(sf::Vector3f(px + tile_width, py, 0.0f), sp::Vector2f((u + 1) * fu, (v + 1) * fv));
            vertices.emplace_back(sf::Vector3f(px + tile_width, py + tile_height, 0.0f), sp::Vector2f((u + 1) * fu, v * fv));
        }
    }
    
    if (!render_data.mesh)
        render_data.mesh = MeshData::create(std::move(vertices));
    else
        render_data.mesh->update(std::move(vertices));
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
        for(; x<w && collision[x][y].up; x++)
        {
            collision[x][y].up = false;
        }
        path.emplace_back(x * tilemap.tile_width, (y + 1) * tilemap.tile_height);
        if (collision[x - 1][y].right)
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
        if (x > 0 && y < w && collision[x - 1][y].down)
        {
            buildPathDown(path, x - 1, y);
            return;
        }
    }

    void buildPathRight(collision::Chains2D::Path& path, int x, int y)
    {
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
    setCollisionShape(builder.result);
}

};//!namespace sp
