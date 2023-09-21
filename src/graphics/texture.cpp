#include <sp2/graphics/texture.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/opengl.h>
#include <sp2/logging.h>


namespace sp {

OpenGLTexture::OpenGLTexture(Type type, const string& name)
: Texture(type, name)
{
    handle = 0;
    smooth = texture_manager.isDefaultSmoothFiltering();
}

OpenGLTexture::~OpenGLTexture()
{
    if (handle)
        glDeleteTextures(1, &handle);
}

void OpenGLTexture::bind()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (image.getSize().x > 0)
    {
        if (type == Type::Static)
            LOG(Info, "Loaded texture:", name, image.getSize().x, "x", image.getSize().y);

        if (handle == 0)
        {
            glGenTextures(1, &handle);
            glBindTexture(GL_TEXTURE_2D, handle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, handle);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.getSize().x, image.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPtr());

        image.clear();
        revision++;
        return;
    }
    
    if (handle)
        glBindTexture(GL_TEXTURE_2D, handle);
    else
        glBindTexture(GL_TEXTURE_2D, 0); //TODO: Fallback texture
}

void OpenGLTexture::setImage(Image&& image)
{
    std::lock_guard<std::mutex> lock(mutex);
    this->image = std::move(image);
}

}//namespace sp
