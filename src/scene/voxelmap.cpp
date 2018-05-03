#include <sp2/scene/voxelmap.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/assert.h>


namespace sp {

Voxelmap::Voxelmap(P<Node> parent, string texture, float voxel_size, int texture_tile_count)
: Voxelmap(parent, texture, voxel_size, texture_tile_count, texture_tile_count)
{
}

Voxelmap::Voxelmap(P<Node> parent, string texture, float voxel_size, int texture_tile_count_x, int texture_tile_count_y)
: sp::Node(parent), voxel_size(voxel_size), texture_tile_count_x(texture_tile_count_x), texture_tile_count_y(texture_tile_count_y)
{
    render_data.shader = Shader::get("internal:basic_shaded.shader");
    render_data.texture = textureManager.get(texture);
    render_data.type = RenderData::Type::Normal;
    render_data.order = -1;
}

void Voxelmap::setVoxel(sp::Vector3i position, int index)
{
    sp2assert(position.x >= 0 && position.y >= 0 && position.z >= 0, "Voxel position needs to be positive");
    sp2assert(index >= -1 && index < int(voxel_data.size()), "Index must be -1 or set in the VoxelData");
    
    sp::Vector3i old_size = getSize();
    sp::Vector3i new_size(std::max(old_size.x, position.x + 1), std::max(old_size.y, position.y + 1), std::max(old_size.z, position.z + 1));
    
    if (old_size.z < new_size.z)
    {
        voxels.resize(new_size.z);
        for(int z=old_size.z; z<new_size.z; z++)
        {
            voxels[z].resize(new_size.y);
            for(int y=0; y<new_size.y; y++)
                voxels[z][y].resize(new_size.x);
        }
    }
    if (old_size.y < new_size.y)
    {
        for(int z=0; z<old_size.z; z++)
        {
            voxels[z].resize(new_size.y);
            for(int y=old_size.y; y<new_size.y; y++)
                voxels[z][y].resize(new_size.x);
        }
    }
    if (old_size.x < new_size.x)
    {
        for(int z=0; z<old_size.z; z++)
        {
            for(int y=0; y<old_size.y; y++)
                voxels[z][y].resize(new_size.x);
        }
    }
    voxels[position.z][position.y][position.x].index = index;
    dirty = true;
}

void Voxelmap::setVoxelData(int index, const Data& data)
{
    sp2assert(index >= 0, "Voxel data index needs to be positive");
    if (int(voxel_data.size()) <= index)
        voxel_data.resize(index + 1);
    voxel_data[index] = data;
}

Vector3i Voxelmap::getSize()
{
    if (voxels.size() < 1)
        return Vector3i(0, 0, 0);
    return Vector3i(voxels[0][0].size(), voxels[0].size(), voxels.size());
}

bool Voxelmap::isSolid(sp::Vector3i position)
{
    if (position.x < 0 || position.y < 0 || position.z < 0)
        return false;
    sp::Vector3i size = getSize();
    if (position.x >= size.x || position.y >= size.y || position.z >= size.z)
        return false;
    Voxel& v = voxels[position.z][position.y][position.x];
    if (v.index < 0)
        return false;
    Data& d = voxel_data[v.index];
    return d.solid;
}

void Voxelmap::onFixedUpdate()
{
    if (!dirty)
        return;
    dirty = false;
    updateMesh();
}

void Voxelmap::updateMesh()
{
    float fu = 1.0 / float(texture_tile_count_x);
    float fv = 1.0 / float(texture_tile_count_y);

    float u_offset = 0.0;
    float v_offset = 0.0;

    MeshData::Vertices vertices;
    MeshData::Indices indices;

    sp::Vector3i size = getSize();
    for(int z=0; z<size.z; z++)
    {
        for(int y=0; y<size.y; y++)
        {
            for(int x=0; x<size.x; x++)
            {
                Voxel& v = voxels[z][y][x];
                if (v.index < 0)
                    continue;
                Data& d = voxel_data[v.index];
                Vector3f p0(x * voxel_size, y * voxel_size, z * voxel_size);
                
                if (d.up_tile > -1 && !isSolid(sp::Vector3i(x, y, z + 1)))
                {
                    int u = d.up_tile % texture_tile_count_x;
                    int v = d.up_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(0, 0, 1);
                    vertices.emplace_back(p0 + Vector3f(0, 0, voxel_size - d.offset.z * voxel_size), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, 0, voxel_size - d.offset.z * voxel_size), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(0, voxel_size, voxel_size - d.offset.z * voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, voxel_size, voxel_size - d.offset.z * voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
                if (d.down_tile > -1 && !isSolid(sp::Vector3i(x, y, z - 1)))
                {
                    int u = d.down_tile % texture_tile_count_x;
                    int v = d.down_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(0, 0, -1);
                    vertices.emplace_back(p0 + Vector3f(0, voxel_size, d.offset.z * voxel_size), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, voxel_size, d.offset.z * voxel_size), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(0, 0, d.offset.z * voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, 0, d.offset.z * voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
                if (d.front_tile > -1 && !isSolid(sp::Vector3i(x, y - 1, z)))
                {
                    int u = d.front_tile % texture_tile_count_x;
                    int v = d.front_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(0, -1, 0);
                    vertices.emplace_back(p0 + Vector3f(0, d.offset.y * voxel_size, 0), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, d.offset.y * voxel_size, 0), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(0, d.offset.y * voxel_size, voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, d.offset.y * voxel_size, voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
                if (d.back_tile > -1 && !isSolid(sp::Vector3i(x, y + 1, z)))
                {
                    int u = d.back_tile % texture_tile_count_x;
                    int v = d.back_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(0, 1, 0);
                    vertices.emplace_back(p0 + Vector3f(voxel_size, voxel_size - d.offset.y * voxel_size, 0), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(0, voxel_size - d.offset.y * voxel_size, 0), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size, voxel_size - d.offset.y * voxel_size, voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(0, voxel_size - d.offset.y * voxel_size, voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
                if (d.left_tile > -1 && !isSolid(sp::Vector3i(x - 1, y, z)))
                {
                    int u = d.left_tile % texture_tile_count_x;
                    int v = d.left_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(-1, 0, 0);
                    vertices.emplace_back(p0 + Vector3f(d.offset.x * voxel_size, voxel_size, 0), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(d.offset.x * voxel_size, 0, 0), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(d.offset.x * voxel_size, voxel_size, voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(d.offset.x * voxel_size, 0, voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
                if (d.right_tile > -1 && !isSolid(sp::Vector3i(x + 1, y, z)))
                {
                    int u = d.right_tile % texture_tile_count_x;
                    int v = d.right_tile / texture_tile_count_x;

                    indices.emplace_back(vertices.size() + 0);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 2);
                    indices.emplace_back(vertices.size() + 1);
                    indices.emplace_back(vertices.size() + 3);
                    
                    sp::Vector3f normal(1, 0, 0);
                    vertices.emplace_back(p0 + Vector3f(voxel_size - d.offset.x * voxel_size, 0, 0), normal, Vector2f(u * fu + u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size - d.offset.x * voxel_size, voxel_size, 0), normal, Vector2f((u + 1) * fu - u_offset, (v + 1) * fv - v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size - d.offset.x * voxel_size, 0, voxel_size), normal, Vector2f(u * fu + u_offset, v * fv + v_offset));
                    vertices.emplace_back(p0 + Vector3f(voxel_size - d.offset.x * voxel_size, voxel_size, voxel_size), normal, Vector2f((u + 1) * fu - u_offset, v * fv + v_offset));
                }
            }
        }
    }

    if (!render_data.mesh)
        render_data.mesh = MeshData::create(std::move(vertices), std::move(indices));
    else
        render_data.mesh->update(std::move(vertices), std::move(indices));
}

};//namespace sp
