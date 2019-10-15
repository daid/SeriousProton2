#ifndef SP2_SCENE_TILEMAP_NODE_H
#define SP2_SCENE_TILEMAP_NODE_H

#include <sp2/scene/node.h>

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
    
    void setTile(int x, int y, int index, Collision collision=Collision::Open);
    void setTileZOffset(int x, int y, double z_offset);
    int getTileIndex(int x, int y);//Returns -1 on out of range
    Collision getTileCollision(int x, int y);//Returns Open on out of range
    
    Vector2i getSize();
    
    virtual void onFixedUpdate() override;
private:
    class Tile
    {
    public:
        Tile() : index(-1), z_offset(0), collision(Collision::Open) {}

        int index;
        double z_offset;
        Collision collision;
    };

    float tile_width;
    float tile_height;
    Vector2i texture_tile_count;
    Vector2f texture_spacing;
    Vector2f texture_margin;
    std::vector<std::vector<Tile>> tiles;
    bool dirty;
    
    void updateMesh();
    void updateCollision();
    
    friend class TilemapCollisionBuilder;
};

}//namespace sp

#endif//SP2_SCENE_TILEMAP_NODE_H
