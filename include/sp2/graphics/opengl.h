#ifndef SP2_GRAPHICS_OPENGL_H
#define SP2_GRAPHICS_OPENGL_H

#ifdef __WIN32
#define GL_APIENTRY __stdcall
#else
#define GL_APIENTRY
#endif

#include <sp2/graphics/opengles2.h>
#include <sp2/graphics/openglDesktop.h>

namespace sp {
void initOpenGL();
};//namespace sp

#endif//SP2_GRAPHICS_OPENGL_H
