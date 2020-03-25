#include <sp2/graphics/mesh/obj.h>
#include <sp2/graphics/color.h>
#include <sp2/graphics/image.h>
#include <sp2/graphics/texture.h>
#include <sp2/graphics/opengl.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/stringutil/convert.h>
#include <sp2/math/matrix4x4.h>


namespace sp {

ObjLoader obj_loader;

class ObjTexture : public OpenGLTexture
{
public:
    ObjTexture(const string& name, sp::Image&& image)
    : OpenGLTexture(Type::Static, name)
    {
        setImage(std::move(image));
    }
};

class MtlData
{
public:
    Color ambient, diffuse, specular;
    Vector2f uv;
};

class ObjData
{
public:
    struct Vertex
    {
        int position = -1;
        int uv = -1;
        int normal = -1;
    };
    struct Group
    {
        string name;
        string material;
        std::vector<std::vector<Vertex>> polygons;
    };

    std::vector<Vector3f> positions;
    std::vector<Vector2f> uvs;
    std::vector<Vector3f> normals;

    std::vector<Group> groups;

    std::map<string, MtlData> materials;

    ObjData(const string& resource_name)
    {
        io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
        if (!stream)
        {
            LOG(Warning, "Failed to find", resource_name);
            return;
        }

        groups.emplace_back();
        while(stream->tell() < stream->getSize())
        {
            string line = stream->readLine().strip();
            if (line.startswith("#") || line.length() == 0)
                continue;

            if (line.startswith("v "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 3)
                    positions.emplace_back(stringutil::convert::toFloat(parts[1]), stringutil::convert::toFloat(parts[3]), stringutil::convert::toFloat(parts[2]));
                else if (parts.size() > 2)
                    positions.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, stringutil::convert::toFloat(parts[2]));
                else if (parts.size() > 1)
                    positions.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, 0.0);
                else
                    positions.emplace_back(0.0, 0.0, 0.0);
            }
            else if (line.startswith("vn "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 3)
                    normals.emplace_back(stringutil::convert::toFloat(parts[1]), stringutil::convert::toFloat(parts[3]), stringutil::convert::toFloat(parts[2]));
                else if (parts.size() > 2)
                    normals.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, stringutil::convert::toFloat(parts[2]));
                else if (parts.size() > 1)
                    normals.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, 0.0);
                else
                    normals.emplace_back(0.0, 0.0, 0.0);
            }
            else if (line.startswith("vt "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 2)
                    uvs.emplace_back(stringutil::convert::toFloat(parts[1]), 1.0-stringutil::convert::toFloat(parts[2]));
                else if (parts.size() > 1)
                    uvs.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0);
                else
                    uvs.emplace_back(0.0, 0.0);
            }
            else if (line.startswith("f "))
            {
                std::vector<string> parts = line.split();
                groups.back().polygons.emplace_back();
                for(unsigned int n=1; n<parts.size(); n++)
                {
                    std::vector<string> index_parts = parts[n].split("/");

                    int v_index = 0, vt_index = 0, vn_index = 0;

                    v_index = stringutil::convert::toInt(index_parts[0]);
                    if (index_parts.size() > 1) vt_index = stringutil::convert::toInt(index_parts[1]);
                    if (index_parts.size() > 2) vn_index = stringutil::convert::toInt(index_parts[2]);

                    if (v_index < 0) v_index = positions.size() + v_index;
                    if (vt_index < 0) vt_index = uvs.size() + vt_index;
                    if (vn_index < 0) vn_index = normals.size() + vn_index;

                    groups.back().polygons.back().push_back({v_index - 1, vt_index - 1, vn_index - 1});
                }
            }
            else if (line.startswith("mtllib "))
            {
                loadMaterialFile(resource_name.substr(0, resource_name.rfind("/") + 1) + line.substr(7));
            }
            else if (line.startswith("usemtl "))
            {
                if (groups.back().polygons.size() > 0)
                    groups.emplace_back();
                groups.back().material = line.substr(7);
            }
            else if (line.startswith("g ") || line.startswith("o "))
            {
                if (groups.back().polygons.size() > 0)
                    groups.emplace_back();
                groups.back().name = line.substr(2);
            }
            else if (line.startswith("g ") || line.startswith("o ") || line.startswith("l ") || line.startswith("s ") || line.startswith("usemap "))
            {
                //Ignore lines, smooth shading, texture mapping
            }
            else
            {
                LOG(Debug, "Unknown line in", resource_name, ":", line);
            }
        }
        if (groups.back().polygons.size() < 1)
            groups.pop_back();
    }

    void loadMaterialFile(const string& name)
    {
        io::ResourceStreamPtr mtl_stream = io::ResourceProvider::get(name);
        string mtl_name = "unknown";
        while(mtl_stream && mtl_stream->tell() < mtl_stream->getSize())
        {
            string line = mtl_stream->readLine().strip();
            if (line.startswith("#") || line.length() == 0)
                continue;
            if (line.startswith("newmtl "))
            {
                mtl_name = line.substr(7);
            }
            else if (line.startswith("Ka "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 1) materials[mtl_name].ambient.r = stringutil::convert::toFloat(parts[1]);
                if (parts.size() > 2) materials[mtl_name].ambient.g = stringutil::convert::toFloat(parts[2]);
                if (parts.size() > 3) materials[mtl_name].ambient.b = stringutil::convert::toFloat(parts[3]);
            }
            else if (line.startswith("Kd "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 1) materials[mtl_name].diffuse.r = stringutil::convert::toFloat(parts[1]);
                if (parts.size() > 2) materials[mtl_name].diffuse.g = stringutil::convert::toFloat(parts[2]);
                if (parts.size() > 3) materials[mtl_name].diffuse.b = stringutil::convert::toFloat(parts[3]);
            }
            else if (line.startswith("Ks "))
            {
                std::vector<string> parts = line.split();
                if (parts.size() > 1) materials[mtl_name].specular.r = stringutil::convert::toFloat(parts[1]);
                if (parts.size() > 2) materials[mtl_name].specular.g = stringutil::convert::toFloat(parts[2]);
                if (parts.size() > 3) materials[mtl_name].specular.b = stringutil::convert::toFloat(parts[3]);
            }
            else if (line.startswith("Ns ") || line.startswith("illum "))
            {
                //Ns = specular exponent
                //illum = illumination model
            }
            else
            {
                LOG(Debug, "Unknown line in", name, ":", mtl_name, ":", line);
            }
        }
    }
};

std::shared_ptr<MeshData> ObjLoader::load(const string& resource_name)
{
    ObjData data(resource_name);

    if (mode == Mode::DiffuseMaterialColorToTexture && data.materials.size() > 0)
    {
        std::vector<uint32_t> pixels;
        pixels.resize(data.materials.size() * 2);

        int index = 0;
        for(auto& it : data.materials)
        {
            pixels[index * 2 + 1] = pixels[index * 2] = uint32_t(it.second.diffuse.r * 255) | (uint32_t(it.second.diffuse.g * 255) << 8) | (uint32_t(it.second.diffuse.b * 255) << 16) | (255 << 24);
            it.second.uv = Vector2f(float(index * 2 + 1) / float(pixels.size()), 0);
            index++;
        }

        obj_info[resource_name].texture = new ObjTexture(resource_name + ".texture", sp::Image(Vector2i(pixels.size(), 1), std::move(pixels)));
    }

    MeshData::Vertices vertices;
    MeshData::Indices indices;

    for(const auto& group : data.groups)
    {
        if (group.name.find("[SP2]") >= 0)
        {
            for(const auto& polygon : group.polygons)
            {
                if (polygon.size() != 3)
                    LOG(Warning, "Point definition wrong point count");
                if (polygon.size() >= 3)
                {
                    auto p0 = data.positions[polygon[0].position];
                    auto p1 = data.positions[polygon[1].position];
                    auto p2 = data.positions[polygon[2].position];

                    auto s01 = (p0 - p1).length();
                    auto s12 = (p1 - p2).length();
                    auto s20 = (p2 - p0).length();

                    Vector3f position;
                    Vector3f up;
                    Vector3f forward;
                    if (s01 < s12 && s01 < s20)
                    {
                        forward = p2;
                        position = p0;
                        up = p1;
                        if (s12 < s20)
                            std::swap(position, up);
                    }
                    else if(s12 < s01 && s12 < s20)
                    {
                        forward = p0;
                        position = p1;
                        up = p2;
                        if (s01 > s20)
                            std::swap(position, up);
                    }
                    else
                    {
                        forward = p1;
                        position = p0;
                        up = p2;
                        if (s12 < s20)
                            std::swap(position, up);
                    }
                    up -= position;
                    forward -= position;
                    up = up.normalized();
                    forward = forward.normalized();

                    if (forward.dot(up) > 0.001)
                        LOG(Warning, "Triangle for point definition not a 90 degree corner:", group.name);

                    auto rotation = Quaterniond::fromVectorToVector(Vector3d(1, 0, 0), Vector3d(forward));
                    rotation = rotation * Quaterniond::fromVectorToVector(rotation * Vector3d(0, 0, 1), Vector3d(up));

                    Info::Point point;
                    point.name = group.name.substr(group.name.find("[SP2]") + 5);
                    point.position = Vector3d(position);
                    point.rotation = rotation;
                    LOG(Debug, point.name, point.position, point.rotation);
                    LOG(Debug, rotation * Vector3d(1, 0, 0), forward);
                    LOG(Debug, rotation * Vector3d(0, 0, 1), up);
                    obj_info[resource_name].points.push_back(point);
                }
            }
        }
        else
        {
            for(const auto& polygon : group.polygons)
            {
                int indices_start = vertices.size();
                for(const auto& vertex : polygon)
                {
                    MeshData::Vertex v(sp::Vector3f(0, 0, 0));
                    if (vertex.position >= 0 && vertex.position < int(data.positions.size()))
                        v.position = data.positions[vertex.position];
                    if (vertex.uv >= 0 && vertex.uv < int(data.uvs.size()))
                        v.uv = data.uvs[vertex.uv];
                    if (vertex.normal >= 0 && vertex.normal < int(data.normals.size()))
                        v.normal = data.normals[vertex.normal].normalized();

                    if (mode == Mode::DiffuseMaterialColorToNormal)
                        v.normal = Vector3f(data.materials[group.material].diffuse.r, data.materials[group.material].diffuse.g, data.materials[group.material].diffuse.b);
                    else if (mode == Mode::DiffuseMaterialColorToTexture)
                        v.uv = data.materials[group.material].uv;

                    vertices.push_back(v);
                }
                for(unsigned int n=2; n<polygon.size(); n++)
                {
                    indices.push_back(indices_start);
                    indices.push_back(indices_start + n);
                    indices.push_back(indices_start + n - 1);
                }
            }
        }
    }

    LOG(Info, "Loaded:", resource_name, vertices.size(), "vertices");
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}


Texture* ObjLoader::getTextureFor(const string& name)
{
    auto it = obj_info.find(name);
    if (it == obj_info.end())
        return nullptr;
    return it->second.texture;
}

}//namespace sp
