#include <sp2/graphics/mesh/glb.h>
#include <sp2/io/resourceProvider.h>

namespace sp {

std::unordered_map<string, GLBLoader::GLBFile> GLBLoader::fileCache;
std::unordered_map<string, std::shared_ptr<MeshData>> GLBLoader::meshCache;

std::shared_ptr<MeshData> GLBLoader::GLBFile::flatMesh() const {
    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
    for(auto& root : roots) {
        addToFlat(vertices, indices, root, Matrix4x4f::identity());
    }
    return MeshData::create(std::move(vertices), std::move(indices));
}

void GLBLoader::GLBFile::addToFlat(sp::MeshData::Vertices& vertices, sp::MeshData::Indices& indices, const Node& node, Matrix4x4f transform) const {
    transform = transform * Matrix4x4f::translate(Vector3f(node.translation)) * Matrix4x4f::fromQuaternion(Quaternionf(node.rotation));
    auto oldVertexCount = vertices.size();
    vertices.reserve(oldVertexCount + node.vertices.size());
    for(auto& v : node.vertices) {
        vertices.emplace_back(
            transform.multiply(v.position),
            transform.applyDirection(v.normal),
            v.uv
        );
    }
    indices.reserve(indices.size() + node.indices.size());
    for(auto i : node.indices) {
        indices.push_back(i + oldVertexCount);
    }

    for(auto& child : node.children)
        addToFlat(vertices, indices, child, transform);
}

const GLBLoader::GLBFile& GLBLoader::get(string resource_name)
{
    auto it = fileCache.find(resource_name);
    if (it != fileCache.end())
        return it->second;
    fileCache[resource_name] = GLBLoader(resource_name).result;
    return get(resource_name);
}

std::shared_ptr<MeshData> GLBLoader::getMesh(string resource_name)
{
    auto it = meshCache.find(resource_name);
    if (it != meshCache.end())
        return it->second;
    meshCache[resource_name] = get(resource_name).flatMesh();
    return getMesh(resource_name);
}

GLBLoader::GLBLoader(string resource_name)
{
    auto resource = io::ResourceProvider::get(resource_name);

    uint32_t magic;
    resource->read(&magic, sizeof(magic));
    if (magic != 0x46546C67) return;
    uint32_t version;
    uint32_t length;
    resource->read(&version, sizeof(version));
    resource->read(&length, sizeof(length));

    while(resource->tell() != resource->getSize()) {
        uint32_t chunkLen;
        resource->read(&chunkLen, sizeof(chunkLen));
        uint32_t chunkType;
        resource->read(&chunkType, sizeof(chunkType));

        if (chunkType == 0x4E4F534A) {
            std::string json_string;
            json_string.resize(chunkLen);
            resource->read(json_string.data(), chunkLen);
            json = nlohmann::json::parse(json_string, nullptr, false);
        } else if (chunkType == 0x004E4942) {
            bindata.resize(chunkLen);
            resource->read(bindata.data(), chunkLen);
        } else {
            resource->seek(resource->tell() + chunkLen);
        }
    }

    for(auto& scene : json["scenes"]) {
        for(auto& node_id : scene["nodes"]) {
            result.roots.emplace_back();
            handleNode(static_cast<int>(node_id), result.roots.back());
        }
    }
}

void GLBLoader::handleNode(int node_id, GLBFile::Node& node)
{
    auto& node_json = json["nodes"][node_id];
    if (node_json.find("translation") != node_json.end()) {
        node.translation.x = node_json["translation"][0];
        node.translation.y = node_json["translation"][1];
        node.translation.z = node_json["translation"][2];
        node.translation = swap_axis(node.translation);
    }
    auto& mesh = json["meshes"][static_cast<int>(node_json["mesh"])];
    for(auto& primitive : mesh["primitives"]) {
        auto& p_a = json["accessors"][static_cast<int>(primitive["attributes"]["POSITION"])];
        auto& p_b = json["bufferViews"][static_cast<int>(p_a["bufferView"])];
        auto& n_a = json["accessors"][static_cast<int>(primitive["attributes"]["NORMAL"])];
        auto& n_b = json["bufferViews"][static_cast<int>(n_a["bufferView"])];
        auto& t_a = json["accessors"][static_cast<int>(primitive["attributes"]["TEXCOORD_0"])];
        auto& t_b = json["bufferViews"][static_cast<int>(t_a["bufferView"])];
        auto& i_a = json["accessors"][static_cast<int>(primitive["indices"])];
        auto& i_b = json["bufferViews"][static_cast<int>(i_a["bufferView"])];
        
        node.vertices.reserve(p_a.value("count", 0));
        for(int vertex_offset=0; vertex_offset<p_a.value("count", 0); vertex_offset++) {
            node.vertices.emplace_back(
                swap_axis(*reinterpret_cast<sp::Vector3f*>(bindata.data() + p_b.value("byteOffset", 0) + p_b.value("byteStride", sizeof(sp::Vector3f)) * vertex_offset)),
                swap_axis(*reinterpret_cast<sp::Vector3f*>(bindata.data() + n_b.value("byteOffset", 0) + n_b.value("byteStride", sizeof(sp::Vector3f)) * vertex_offset)),
                *reinterpret_cast<sp::Vector2f*>(bindata.data() + t_b.value("byteOffset", 0) + t_b.value("byteStride", sizeof(sp::Vector2f)) * vertex_offset)
            );
        }
        auto buffer = bindata.data() + i_b.value("byteOffset", 0);
        node.indices.reserve(i_a.value("count", 0));
        for(int indice_offset=0; indice_offset<i_a.value("count", 0); indice_offset++) {
            int index = 0;
            switch(static_cast<int>(i_a["componentType"])) {
            case 5121:
                index = buffer[indice_offset];
                break;
            case 5123:
                index = buffer[indice_offset*2] | (buffer[indice_offset*2+1] << 8);
                break;
            }
            node.indices.push_back(index);
        }
    }

    node.name = node_json.value("name", "no-name");

    if (node_json.find("children") != node_json.end()) {
        for(auto& child_id : node_json["children"]) {
            node.children.emplace_back();
            handleNode(static_cast<int>(child_id), node.children.back());
        }
    }
}

}//namespace sp
