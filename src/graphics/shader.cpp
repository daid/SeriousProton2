#include <sp2/graphics/shader.h>
#include <sp2/graphics/opengl.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/logging.h>

namespace sp {

std::map<string, Shader*> Shader::cached_shaders;

Shader* Shader::get(string name)
{
    auto it = cached_shaders.find(name);
    if (it != cached_shaders.end())
    {
        return it->second;
    }
    
    Shader* new_shader = new Shader();
    io::ResourceStreamPtr stream = io::ResourceProvider::get(name);
    if (stream)
    {
        LOG(Info, "Loading shader:", name);
        string vertex_shader;
        string fragment_shader;
        int type = -1;
        while(stream->tell() != stream->getSize())
        {
            string line = stream->readLine();
            if (line == "[VERTEX]")
                type = 1;
            else if (line == "[FRAGMENT]")
                type = 2;
            else if (type == 1)
                vertex_shader += line + "\n";
            else if (type == 2)
                fragment_shader += line + "\n";
        }
        new_shader->loadFromMemory(vertex_shader, fragment_shader);
    }
    else
    {
        LOG(Warning, "Failed to find shader:", name);
    }
    cached_shaders[name] = new_shader;
    return new_shader;
}

Shader::Shader()
{
}

};//!namespace sp
