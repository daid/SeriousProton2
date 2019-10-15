#include <sp2/graphics/mesh/fbx.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/math/matrix4x4.h>
#include "miniz.h"

namespace sp {

class FbxProperty
{
public:
    std::vector<double> numbers;
    string str;

    void read(io::ResourceStreamPtr stream)
    {
        char property_type;
        stream->read(&property_type, 1);
        switch(property_type)
        {
        case 'C': { uint8_t b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'Y': { int16_t b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'I': { int32_t b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'L': { int64_t b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'F': { float b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'D': { double b; stream->read(&b, sizeof(b)); numbers.emplace_back(b);} break;
        case 'S':
        case 'R': {
            int32_t length;
            stream->read(&length, 4);
            char buffer[length + 1];
            stream->read(buffer, length);
            buffer[length] = 0;
            str = buffer;
            }break;
        case 'b': readArray<uint8_t>(stream); break;
        case 'i': readArray<int32_t>(stream); break;
        case 'l': readArray<int64_t>(stream); break;
        case 'f': readArray<float>(stream); break;
        case 'd': readArray<double>(stream); break;
        default:
            LOG(Debug, "Unknown property type:", property_type);
        }
    }
    
    template<typename T> void readArray(io::ResourceStreamPtr stream)
    {
        int32_t element_count;
        int32_t encoding;
        uint32_t encoded_size;
        stream->read(&element_count, 4);
        stream->read(&encoding, 4);
        stream->read(&encoded_size, 4);
        numbers.reserve(element_count);
        if (encoding)
        {
            uint8_t buffer[encoded_size];
            stream->read(buffer, encoded_size);
            std::vector<T> tmp;
            tmp.resize(element_count);
            unsigned long size = element_count * sizeof(T);
            mz_uncompress(reinterpret_cast<unsigned char*>(tmp.data()), &size, buffer, encoded_size);
            for(int n=0; n<element_count; n++)
                numbers.emplace_back(tmp[n]);
        }
        else
        {
            for(int n=0; n<element_count; n++)
            {
                T b;
                stream->read(&b, sizeof(b));
                numbers.emplace_back(b);
            }
        }
    }
};

class FbxNode
{
public:
    string name;
    std::vector<FbxProperty> properties;
    std::vector<FbxNode> children;
    
    FbxNode* find(const string& name)
    {
        for(auto& child : children)
            if (child.name == name)
                return &child;
        return nullptr;
    }
    
    std::vector<FbxNode*> findAll(const string& name)
    {
        std::vector<FbxNode*> result;
        for(auto& child : children)
            if (child.name == name)
                result.push_back(&child);
        return result;
    }
    
    FbxNode* getProperty70(const string& name)
    {
        FbxNode* properties_root = find("Properties70");
        if (!properties_root)
            return nullptr;
        for(FbxNode& child : properties_root->children)
        {
            if (child.properties.size() > 0 && child.properties[0].str == name)
            {
                return &child;
            }
        }
        return nullptr;
    }
    
    sp::Vector3d getProperty70asVector3d(const string& name, sp::Vector3d default_value=sp::Vector3d(0,0,0))
    {
        FbxNode* node = getProperty70(name);
        if (!node)
            return default_value;
        int idx = node->properties.size() - 3;
        return sp::Vector3d(node->properties[idx].numbers[0], node->properties[idx+1].numbers[0], node->properties[idx+2].numbers[0]);
    }
};

class BinaryFbxReader
{
public:
    BinaryFbxReader(const string& resource_name)
    {
        stream = io::ResourceProvider::get(resource_name);
        
        char header[21];
        uint16_t unknown;
        stream->read(header, 21);
        stream->read(&unknown, 2);
        stream->read(&version, 4);
        
        while(true)
        {
            FbxNode child;
            if (!readNode(child))
                break;
            root.children.push_back(child);
        }
    }

    bool readNode(FbxNode& node)
    {
        int64_t end_offset, num_properties, properties_list_len;
        if (version < 7500)
        {
            uint32_t end_offset32, num_properties32, properties_list_len32;
            stream->read(&end_offset32, 4);
            stream->read(&num_properties32, 4);
            stream->read(&properties_list_len32, 4);
            end_offset = end_offset32;
            num_properties = num_properties32;
            properties_list_len = properties_list_len32;
        }
        else
        {
            stream->read(&end_offset, 8);
            stream->read(&num_properties, 8);
            stream->read(&properties_list_len, 8);
        }
        uint8_t name_len;
        stream->read(&name_len, 1);

        if (end_offset >= stream->getSize() || end_offset == 0)
            return false;
        
        char name[name_len + 1];
        stream->read(name, name_len);
        name[name_len] = 0;
        node.name = name;
        node.properties.resize(num_properties);
        
        for(int n=0; n<num_properties; n++)
        {
            node.properties[n].read(stream);
        }
        
        while(stream->tell() < end_offset && stream->tell() < stream->getSize())
        {
            FbxNode child;
            if (readNode(child))
                node.children.push_back(child);
        }
        return true;
    }

    FbxNode root;

private:
    io::ResourceStreamPtr stream;
    uint32_t version;
};

std::shared_ptr<MeshData> FbxLoader::load(const string& resource_name)
{
    MeshData::Vertices vertices;
    MeshData::Indices indices;

    BinaryFbxReader reader(resource_name);
    FbxNode* root = &reader.root;

    std::map<int, Matrix4x4d> id_to_matrix;
    for(FbxNode* model : root->find("Objects")->findAll("Model"))
    {
        int id = int(model->properties[0].numbers[0]);
        sp::Vector3d translation = model->getProperty70asVector3d("Lcl Translation");
        sp::Vector3d rotation = model->getProperty70asVector3d("Lcl Rotation");
        
        id_to_matrix[id] = Matrix4x4d::translate(translation) * Matrix4x4d::rotate(rotation.x, 1, 0, 0) * Matrix4x4d::rotate(rotation.y, 0, 1, 0) * Matrix4x4d::rotate(rotation.z, 0, 0, 1);
    }
    std::map<int, int> child_parent_relations;
    for(FbxNode* connection : root->find("Connections")->findAll("C"))
    {
        if (connection->properties[0].str != "OO")
            continue;
        child_parent_relations[int(connection->properties[1].numbers[0])] = int(connection->properties[2].numbers[0]);
    }

    for(FbxNode* geometry : root->find("Objects")->findAll("Geometry"))
    {
        int id = geometry->properties[0].numbers[0];
        Matrix4x4d transform = Matrix4x4d::identity();
        while(child_parent_relations.find(id) != child_parent_relations.end())
        {
            id = child_parent_relations[id];
            if (id_to_matrix.find(id) != id_to_matrix.end())
                transform = id_to_matrix[id] * transform;
        }
    
        std::vector<double>& vertex_data = geometry->find("Vertices")->properties[0].numbers;
        std::vector<double>& normal_data = geometry->find("LayerElementNormal")->find("Normals")->properties[0].numbers;
        std::vector<double>& uv_data = geometry->find("LayerElementUV")->find("UV")->properties[0].numbers;
        std::vector<double>& uv_index_data = geometry->find("LayerElementUV")->find("UVIndex")->properties[0].numbers;
        std::vector<double>& index_data = geometry->find("PolygonVertexIndex")->properties[0].numbers;
        
        int idx0 = -1;
        int idx1 = -1;
        for(unsigned int n=0; n<index_data.size(); n++)
        {
            int vertex_idx = index_data[n];
            if (vertex_idx < 0)
                vertex_idx = -vertex_idx-1;
            int uv_index = uv_index_data[n];

            sp::Vector3d v = transform * sp::Vector3d(vertex_data[vertex_idx*3], vertex_data[vertex_idx*3+1], vertex_data[vertex_idx*3+2]);
            sp::Vector3d normal = transform.applyDirection(sp::Vector3d(normal_data[n*3], normal_data[n*3+1], normal_data[n*3+2]));
            vertices.emplace_back(
                sp::Vector3f(v.x, v.z, v.y),
                sp::Vector3f(normal.x, normal.z, normal.y),
                sp::Vector2f(uv_data[uv_index*2], 1.0-uv_data[uv_index*2+1]));

            if (idx0 > -1 && idx1 > -1)
            {
                indices.push_back(idx1);
                indices.push_back(idx0);
                indices.push_back(vertices.size() - 1);
            }
            if (idx0 == -1) idx0 = idx1;
            idx1 = vertices.size() - 1;
            if (index_data[n] < 0)
            {
                idx0 = idx1 = -1;
            }
        }
    }
    
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}

}//namespace sp
