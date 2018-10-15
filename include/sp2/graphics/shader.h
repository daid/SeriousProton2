#ifndef SP2_GRAPHICS_SHADER_H
#define SP2_GRAPHICS_SHADER_H

#include <sp2/string.h>
#include <sp2/math/vector.h>
#include <sp2/math/matrix4x4.h>
#include <sp2/graphics/color.h>
#include <sp2/nonCopyable.h>

namespace sp {

class Texture;

class Shader : public NonCopyable
{
public:    
    void bind();
    void setUniform(const string& s, const Matrix4x4f& matrix);
    void setUniform(const string& s, const Matrix4x4d& matrix);
    void setUniform(const string& s, const Vector2f& v);
    void setUniform(const string& s, const Vector3f& v);
    void setUniform(const string& s, const Color& v);
    void setUniform(const string& s, Texture* v);

private:
    Shader(string name);
    Shader(string name, string&& vertex_shader, string&& fragment_shader);
    ~Shader();
    
    unsigned int compileShader(const char* code, int type);
    int getUniformLocation(const string& s);

    unsigned int program;
    std::map<string, int> uniform_mapping;

    string name;
    string vertex_shader;
    string fragment_shader;
public:
    static Shader* get(string name);
    static void unbind();
    
private:
    static std::map<string, Shader*> cached_shaders;
    static Shader* bound_shader;
};

};//namespace sp


#endif//SP2_GRAPHICS_SHADER_H
