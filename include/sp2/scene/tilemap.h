#ifndef SP2_SCENE_TILEMAP_NODE_H
#define SP2_SCENE_TILEMAP_NODE_H

#include <sp2/scene/node.h>
#include <sp2/container/infinigrid.h>

namespace sp {

class Tilemap : public Node
{
public:
    enum class Collision
    {
        Open,
        Solid,
        Platform
    };
    static constexpr int flip_horizontal = 0x100000;
    static constexpr int flip_vertical = 0x200000;
    static constexpr int flip_diagonal = 0x400000;

    Tilemap(P<Node> parent, const string& texture, float tile_size, int texture_tile_count);
    Tilemap(P<Node> parent, const string& texture, float tile_width, float tile_height, int texture_tile_count_x, int texture_tile_count_y);
    
    void setTilemapSpacingMargin(float spacing, float margin);
    
    void setTile(sp::Vector2i position, int index, Collision collision);
    void setTile(sp::Vector2i position, int index, double z_offset=0.0, sp::Vector3f normal={0,0,1}, Collision collision=Collision::Open);
    int getTileIndex(sp::Vector2i position);//Returns -1 on out of range
    Collision getTileCollision(sp::Vector2i position);//Returns Open on out of range
    
    virtual void onFixedUpdate() override;
private:
    struct Tile
    {
        int index;
        double z_offset;
        sp::Vector3f normal;
        Collision collision;
    };

    float tile_width;
    float tile_height;
    Vector2i texture_tile_count;
    Vector2f texture_spacing;
    Vector2f texture_margin;
    InfiniGrid<Tile> tiles;
    bool dirty;
    
    void updateMesh();
    void updateCollision();
    
    friend class TilemapCollisionBuilder;
};

}//namespace sp

#endif//SP2_SCENE_TILEMAP_NODE_H
