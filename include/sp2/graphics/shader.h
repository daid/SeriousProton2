#ifndef SP2_GRAPHICS_SHADER_H
#define SP2_GRAPHICS_SHADER_H

#include <sp2/string.h>
#include <SFML/Graphics/Shader.hpp>

namespace sp {

class Shader : public sf::Shader
{
public:
    static Shader* get(string name);
    
private:
    static std::map<string, Shader*> cached_shaders;

    Shader();
};

};//!namespace sp


#endif//SP2_GRAPHICS_SHADER_H
