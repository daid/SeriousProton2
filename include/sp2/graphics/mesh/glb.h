#ifndef SP2_GRAPHICS_MESH_GLB_H
#define SP2_GRAPHICS_MESH_GLB_H

#include <sp2/graphics/meshdata.h>
#include <sp2/math/matrix4x4.h>
#include <nlohmann/json.hpp>


namespace sp {

class GLBLoader
{
public:
    class GLBFile {
    public:
        struct Node {
            string name;

            sp::Vector3d translation{};
            sp::Quaterniond rotation{};
            sp::MeshData::Vertices vertices;
            sp::MeshData::Indices indices;

            std::vector<Node> children;
        };
        std::vector<Node> roots;

        std::shared_ptr<MeshData> flatMesh() const;
    private:
        void addToFlat(sp::MeshData::Vertices& vertices, sp::MeshData::Indices& indices, const Node& node, Matrix4x4f transform) const;
    };

    static const GLBFile& get(string resource_name);
    static std::shared_ptr<MeshData> getMesh(string resource_name);
private:
    GLBLoader(string resource_name);
    void handleNode(int node_id, GLBFile::Node& node);

    template<typename T> static sp::Vector3<T> swap_axis(sp::Vector3<T> v) {
       return {-v.x, v.z, v.y};
    }

    GLBFile result;
    nlohmann::json json;
    std::vector<uint8_t> bindata;

    static std::unordered_map<string, GLBFile> fileCache;
    static std::unordered_map<string, std::shared_ptr<MeshData>> meshCache;
};

}//namespace sp

#endif//SP2_GRAPHICS_MESH_GLB_H
