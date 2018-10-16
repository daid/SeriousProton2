#include <sp2/graphics/texture.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/opengl.h>
#include <sp2/logging.h>


namespace sp {

OpenGLTexture::OpenGLTexture(Type type, string name)
: Texture(type, name)
{
    handle = 0;
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
        LOG(Info, "Loaded image", name, image.getSize().x, "x", image.getSize().y);

        if (handle == 0)
        {
            glGenTextures(1, &handle);
            glBindTexture(GL_TEXTURE_2D, handle);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_manager.default_smooth ? GL_LINEAR : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_manager.default_smooth ? GL_LINEAR : GL_NEAREST);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, handle);
        }
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

};//namespace sp
