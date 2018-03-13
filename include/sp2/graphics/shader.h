#ifndef SP2_GRAPHICS_SHADER_H
#define SP2_GRAPHICS_SHADER_H

#include <sp2/string.h>
#include <sp2/math/vector.h>
#include <SFML/Graphics/Shader.hpp>

namespace sp {

class Shader : public sf::Shader
{
public:
    static Shader* get(string name);
    
    void setUniformTmp(const string& name, const Vector2f v) { setUniform(name, sf::Vector2f(v.x, v.y)); }
private:
    static std::map<string, Shader*> cached_shaders;

    Shader();
};

};//!namespace sp


#endif//SP2_GRAPHICS_SHADER_H
