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

class MtlData
{
public:
    Color ambient, diffuse, specular;
    Vector2f uv;
};

class ObjTexture : public OpenGLTexture
{
public:
    ObjTexture(const string& name, sp::Image&& image)
    : OpenGLTexture(Type::Static, name)
    {
        setImage(std::move(image));
    }
};

std::shared_ptr<MeshData> ObjLoader::load(const string& resource_name)
{
    io::ResourceStreamPtr stream = io::ResourceProvider::get(resource_name);
    if (!stream)
    {
        LOG(Warning, "Failed to find", resource_name);
        return nullptr;
    }

    MeshData::Vertices vertices;
    MeshData::Indices indices;
    
    std::vector<Vector3f> v_list;
    std::vector<Vector2f> vt_list;
    std::vector<Vector3f> vn_list;
    
    std::map<string, MtlData> materials;
    string active_material = "unknown";
    sp::Image generated_texture;
    
    while(stream->tell() < stream->getSize())
    {
        string line = stream->readLine().strip();
        if (line.startswith("#") || line.length() == 0)
            continue;
        
        if (line.startswith("v "))
        {
            std::vector<string> parts = line.split();
            if (parts.size() > 3)
                v_list.emplace_back(stringutil::convert::toFloat(parts[1]), stringutil::convert::toFloat(parts[3]), stringutil::convert::toFloat(parts[2]));
            else if (parts.size() > 2)
                v_list.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, stringutil::convert::toFloat(parts[2]));
            else if (parts.size() > 1)
                v_list.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, 0.0);
            else
                v_list.emplace_back(0.0, 0.0, 0.0);
        }
        else if (line.startswith("vn "))
        {
            std::vector<string> parts = line.split();
            if (parts.size() > 3)
                vn_list.emplace_back(stringutil::convert::toFloat(parts[1]), stringutil::convert::toFloat(parts[3]), stringutil::convert::toFloat(parts[2]));
            else if (parts.size() > 2)
                vn_list.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, stringutil::convert::toFloat(parts[2]));
            else if (parts.size() > 1)
                vn_list.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0, 0.0);
            else
                vn_list.emplace_back(0.0, 0.0, 0.0);
        }
        else if (line.startswith("vt "))
        {
            std::vector<string> parts = line.split();
            if (parts.size() > 2)
                vt_list.emplace_back(stringutil::convert::toFloat(parts[1]), 1.0-stringutil::convert::toFloat(parts[2]));
            else if (parts.size() > 1)
                vt_list.emplace_back(stringutil::convert::toFloat(parts[1]), 0.0);
            else
                vt_list.emplace_back(0.0, 0.0);
        }
        else if (line.startswith("f "))
        {
            std::vector<string> parts = line.split();
            int indices_start = vertices.size();
            for(unsigned int n=1; n<parts.size(); n++)
            {
                std::vector<string> index_parts = parts[n].split("/");
                
                int v_index = 0, vt_index = 0, vn_index = 0;
                
                v_index = stringutil::convert::toInt(index_parts[0]);
                if (index_parts.size() > 1) vt_index = stringutil::convert::toInt(index_parts[1]);
                if (index_parts.size() > 2) vn_index = stringutil::convert::toInt(index_parts[2]);
                
                if (v_index < 0) v_index = v_list.size() + v_index;
                if (vt_index < 0) vt_index = vt_list.size() + vt_index;
                if (vn_index < 0) vn_index = vn_list.size() + vn_index;
                
                MeshData::Vertex v(sp::Vector3f(0, 0, 0));
                if (v_index > 0 && v_index <= int(v_list.size()))
                    v.position = v_list[v_index - 1];
                if (vt_index > 0 && vt_index <= int(vt_list.size()))
                    v.uv = vt_list[vt_index - 1];
                if (vn_index > 0 && vn_index <= int(vn_list.size()))
                    v.normal = vn_list[vn_index - 1].normalized();
                
                if (mode == Mode::DiffuseMaterialColorToNormal)
                    v.normal = Vector3f(materials[active_material].diffuse.r, materials[active_material].diffuse.g, materials[active_material].diffuse.b);
                else if (mode == Mode::DiffuseMaterialColorToTexture)
                    v.uv = materials[active_material].uv;
                
                vertices.push_back(v);
            }
            for(unsigned int n=3; n<parts.size(); n++)
            {
                indices.push_back(indices_start);
                indices.push_back(indices_start + n - 1);
                indices.push_back(indices_start + n - 2);
            }
        }
        else if (line.startswith("mtllib "))
        {
            string mtl_resource_name = resource_name.substr(0, resource_name.rfind("/") + 1) + line.substr(7);
            
            io::ResourceStreamPtr mtl_stream = io::ResourceProvider::get(mtl_resource_name);
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
                    LOG(Debug, "Unknown line in", mtl_resource_name, ":", mtl_name, ":", line);
                }
            }
            
            if (materials.size() && mode == Mode::DiffuseMaterialColorToTexture)
            {
                uint32_t pixels[materials.size() * 2];
                
                int index = 0;
                for(auto& it : materials)
                {
                    pixels[index * 2 + 1] = pixels[index * 2] = uint32_t(it.second.diffuse.r * 255) | (uint32_t(it.second.diffuse.g * 255) << 8) | (uint32_t(it.second.diffuse.b * 255) << 16) | (255 << 24);
                    it.second.uv = Vector2f(float(index * 2 + 1) / float(materials.size() * 2), 0);
                    index++;
                }
                
                generated_texture.update(Vector2i(materials.size() * 2, 1), pixels);
            }
        }
        else if (line.startswith("usemtl "))
        {
            active_material = line.substr(7);
        }
        else if (line.startswith("g ") || line.startswith("o ") || line.startswith("l ") || line.startswith("s ") || line.startswith("usemap "))
        {
            //Ignore groups, object name, lines, smooth shading, texture mapping
        }
        else
        {
            LOG(Debug, "Unknown line in", resource_name, ":", line);
        }
    }
    
    if (mode == Mode::DiffuseMaterialColorToTexture)
    {
        texture_cache[resource_name] = new ObjTexture(resource_name + ".texture", std::move(generated_texture));
    }
    LOG(Info, "Loaded:", resource_name, vertices.size(), "vertices");
    return std::make_shared<MeshData>(std::move(vertices), std::move(indices));
}


Texture* ObjLoader::getTextureFor(const string& name)
{
    auto it = texture_cache.find(name);
    if (it == texture_cache.end())
        return nullptr;
    return it->second;
}

}//namespace sp
