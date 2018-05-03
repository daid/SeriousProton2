#ifndef SP2_SCENE_VOXELMAP_NODE_H
#define SP2_SCENE_VOXELMAP_NODE_H

#include <sp2/scene/node.h>

namespace sp {

class Voxelmap : public Node
{
public:
    class Data
    {
    public:
        int up_tile = -1;
        int down_tile = -1;
        int left_tile = -1;
        int right_tile = -1;
        int front_tile = -1;
        int back_tile = -1;
        
        bool solid = true;
    };

    Voxelmap(P<Node> parent, string texture, float voxel_size, int texture_tile_count);
    Voxelmap(P<Node> parent, string texture, float voxel_size, int texture_tile_count_x, int texture_tile_count_y);

    void setVoxel(sp::Vector3i position, int index);
    void setVoxelData(int index, const Data& data);

    Vector3i getSize();
    bool isSolid(sp::Vector3i position);
    
    virtual void onFixedUpdate();
private:
    class Voxel
    {
    public:
        Voxel() : index(-1) {}

        int index;
    };

    float voxel_size;
    int texture_tile_count_x;
    int texture_tile_count_y;
    std::vector<std::vector<std::vector<Voxel>>> voxels;
    std::vector<Data> voxel_data;
    bool dirty;
    
    void updateMesh();
};

};//namespace sp

#endif//SP2_SCENE_VOXELMAP_NODE_H
