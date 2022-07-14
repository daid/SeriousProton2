#include <sp2/graphics/renderTexture.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/opengl.h>


namespace sp {

RenderTexture::RenderTexture(const string& name, Vector2i size, bool double_buffered)
: Texture(Texture::Type::Dynamic, name), double_buffered(double_buffered), size(size)
{
    flipped = false;
    auto_clear = !double_buffered;
    smooth = texture_manager.isDefaultSmoothFiltering();

    for(int n=0; n<2; n++)
    {
        dirty[n] = false;

        frame_buffer[n] = 0;
        color_buffer[n] = 0;
        depth_buffer[n] = 0;
        stencil_buffer[n] = 0;
    }
}

RenderTexture::~RenderTexture()
{
    int count = double_buffered ? 2 : 1;
    if (frame_buffer[0])
    {
        glDeleteFramebuffers(count, frame_buffer);
        glDeleteTextures(count, color_buffer);
        glDeleteRenderbuffers(count, depth_buffer);
    }
}

void RenderTexture::create()
{
    int count = double_buffered ? 2 : 1;

    if (!frame_buffer[0])
    {
        glGenFramebuffers(count, frame_buffer);
        glGenTextures(count, color_buffer);
        glGenRenderbuffers(count, depth_buffer);
    }

    for(int n=0; n<count; n++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer[n]);

        glBindTexture(GL_TEXTURE_2D, color_buffer[n]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buffer[n], 0);

        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer[n]);
#if defined(GL_DEPTH24_STENCIL8) && defined(GL_DEPTH_STENCIL_ATTACHMENT)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer[n]);
#elif defined(GL_DEPTH_STENCIL) && defined(GL_DEPTH_STENCIL_ATTACHMENT)
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_STENCIL, size.x, size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer[n]);
#else
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, size.x, size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer[n]);
#endif
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            LOG(Error, "Failed to create OpenGL FrameBuffer for RenderTexture!");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderTexture::bind()
{
    int index = 0;
    if (create_buffers)
    {
        create_buffers = false;
        create();
    }

    if (double_buffered)
        index = flipped ? 0 : 1;

    if (dirty[index])
    {
        glFlush(); //Is this needed if we double buffer?
        dirty[index] = false;
    }
    glBindTexture(GL_TEXTURE_2D, color_buffer[index]);
}

void RenderTexture::setSize(Vector2i new_size)
{
    if (size != new_size)
    {
        size = new_size;
        create_buffers = true;
    }
}

Vector2i RenderTexture::getSize() const
{
    return size;
}

void RenderTexture::activateRenderTarget()
{
    if (create_buffers)
    {
        create_buffers = false;
        create();
    }
    if (double_buffered)
        flipped = !flipped;

    int index = flipped ? 1 : 0;

    dirty[index] = true;
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer[index]);
    if (auto_clear)
    {
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
}

}//namespace sp
