#include <sp2/graphics/opengl.h>
#include <sp2/logging.h>
#include <SDL_video.h>

static bool init_done = false;

#ifdef SP2_GRAPHICS_OPENGLES2_H
// OpenGL ES 2.0 functions:
void (GL_APIENTRY * glActiveTexture)(GLenum texture);
void (GL_APIENTRY * glAttachShader)(GLuint program, GLuint shader);
void (GL_APIENTRY * glBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
void (GL_APIENTRY * glBindBuffer)(GLenum target, GLuint buffer);
void (GL_APIENTRY * glBindFramebuffer)(GLenum target, GLuint framebuffer);
void (GL_APIENTRY * glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
void (GL_APIENTRY * glBindTexture)(GLenum target, GLuint texture);
void (GL_APIENTRY * glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (GL_APIENTRY * glBlendEquation)(GLenum mode);
void (GL_APIENTRY * glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
void (GL_APIENTRY * glBlendFunc)(GLenum sfactor, GLenum dfactor);
void (GL_APIENTRY * glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
void (GL_APIENTRY * glBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void (GL_APIENTRY * glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
GLenum (GL_APIENTRY * glCheckFramebufferStatus)(GLenum target);
void (GL_APIENTRY * glClear)(GLbitfield mask);
void (GL_APIENTRY * glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (GL_APIENTRY * glClearDepthf)(GLclampf depth);
void (GL_APIENTRY * glClearStencil)(GLint s);
void (GL_APIENTRY * glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void (GL_APIENTRY * glCompileShader)(GLuint shader);
void (GL_APIENTRY * glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid* data);
void (GL_APIENTRY * glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid* data);
void (GL_APIENTRY * glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
void (GL_APIENTRY * glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GLuint (GL_APIENTRY * glCreateProgram)(void);
GLuint (GL_APIENTRY * glCreateShader)(GLenum type);
void (GL_APIENTRY * glCullFace)(GLenum mode);
void (GL_APIENTRY * glDeleteBuffers)(GLsizei n, const GLuint* buffers);
void (GL_APIENTRY * glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);
void (GL_APIENTRY * glDeleteProgram)(GLuint program);
void (GL_APIENTRY * glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers);
void (GL_APIENTRY * glDeleteShader)(GLuint shader);
void (GL_APIENTRY * glDeleteTextures)(GLsizei n, const GLuint* textures);
void (GL_APIENTRY * glDepthFunc)(GLenum func);
void (GL_APIENTRY * glDepthMask)(GLboolean flag);
void (GL_APIENTRY * glDepthRangef)(GLclampf zNear, GLclampf zFar);
void (GL_APIENTRY * glDetachShader)(GLuint program, GLuint shader);
void (GL_APIENTRY * glDisable)(GLenum cap);
void (GL_APIENTRY * glDisableVertexAttribArray)(GLuint index);
void (GL_APIENTRY * glDrawArrays)(GLenum mode, GLint first, GLsizei count);
void (GL_APIENTRY * glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void (GL_APIENTRY * glEnable)(GLenum cap);
void (GL_APIENTRY * glEnableVertexAttribArray)(GLuint index);
void (GL_APIENTRY * glFinish)(void);
void (GL_APIENTRY * glFlush)(void);
void (GL_APIENTRY * glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
void (GL_APIENTRY * glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void (GL_APIENTRY * glFrontFace)(GLenum mode);
void (GL_APIENTRY * glGenBuffers)(GLsizei n, GLuint* buffers);
void (GL_APIENTRY * glGenerateMipmap)(GLenum target);
void (GL_APIENTRY * glGenFramebuffers)(GLsizei n, GLuint* framebuffers);
void (GL_APIENTRY * glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers);
void (GL_APIENTRY * glGenTextures)(GLsizei n, GLuint* textures);
void (GL_APIENTRY * glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
void (GL_APIENTRY * glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
void (GL_APIENTRY * glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
GLint (GL_APIENTRY * glGetAttribLocation)(GLuint program, const GLchar* name);
void (GL_APIENTRY * glGetBooleanv)(GLenum pname, GLboolean* params);
void (GL_APIENTRY * glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params);
GLenum (GL_APIENTRY * glGetError)(void);
void (GL_APIENTRY * glGetFloatv)(GLenum pname, GLfloat* params);
void (GL_APIENTRY * glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetIntegerv)(GLenum pname, GLint* params);
void (GL_APIENTRY * glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
void (GL_APIENTRY * glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
void (GL_APIENTRY * glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
void (GL_APIENTRY * glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
const GLubyte* (GL_APIENTRY * glGetString)(GLenum name);
void (GL_APIENTRY * glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params);
void (GL_APIENTRY * glGetTexParameteriv)(GLenum target, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetUniformfv)(GLuint program, GLint location, GLfloat* params);
void (GL_APIENTRY * glGetUniformiv)(GLuint program, GLint location, GLint* params);
GLint (GL_APIENTRY * glGetUniformLocation)(GLuint program, const GLchar* name);
void (GL_APIENTRY * glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params);
void (GL_APIENTRY * glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params);
void (GL_APIENTRY * glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer);
void (GL_APIENTRY * glHint)(GLenum target, GLenum mode);
GLboolean (GL_APIENTRY * glIsBuffer)(GLuint buffer);
GLboolean (GL_APIENTRY * glIsEnabled)(GLenum cap);
GLboolean (GL_APIENTRY * glIsFramebuffer)(GLuint framebuffer);
GLboolean (GL_APIENTRY * glIsProgram)(GLuint program);
GLboolean (GL_APIENTRY * glIsRenderbuffer)(GLuint renderbuffer);
GLboolean (GL_APIENTRY * glIsShader)(GLuint shader);
GLboolean (GL_APIENTRY * glIsTexture)(GLuint texture);
void (GL_APIENTRY * glLineWidth)(GLfloat width);
void (GL_APIENTRY * glLinkProgram)(GLuint program);
void (GL_APIENTRY * glPixelStorei)(GLenum pname, GLint param);
void (GL_APIENTRY * glPolygonOffset)(GLfloat factor, GLfloat units);
void (GL_APIENTRY * glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void (GL_APIENTRY * glReleaseShaderCompiler)(void);
void (GL_APIENTRY * glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
void (GL_APIENTRY * glSampleCoverage)(GLclampf value, GLboolean invert);
void (GL_APIENTRY * glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
void (GL_APIENTRY * glShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
void (GL_APIENTRY * glShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
void (GL_APIENTRY * glStencilFunc)(GLenum func, GLint ref, GLuint mask);
void (GL_APIENTRY * glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
void (GL_APIENTRY * glStencilMask)(GLuint mask);
void (GL_APIENTRY * glStencilMaskSeparate)(GLenum face, GLuint mask);
void (GL_APIENTRY * glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
void (GL_APIENTRY * glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
void (GL_APIENTRY * glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void (GL_APIENTRY * glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
void (GL_APIENTRY * glTexParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
void (GL_APIENTRY * glTexParameteri)(GLenum target, GLenum pname, GLint param);
void (GL_APIENTRY * glTexParameteriv)(GLenum target, GLenum pname, const GLint* params);
void (GL_APIENTRY * glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void (GL_APIENTRY * glUniform1f)(GLint location, GLfloat x);
void (GL_APIENTRY * glUniform1fv)(GLint location, GLsizei count, const GLfloat* v);
void (GL_APIENTRY * glUniform1i)(GLint location, GLint x);
void (GL_APIENTRY * glUniform1iv)(GLint location, GLsizei count, const GLint* v);
void (GL_APIENTRY * glUniform2f)(GLint location, GLfloat x, GLfloat y);
void (GL_APIENTRY * glUniform2fv)(GLint location, GLsizei count, const GLfloat* v);
void (GL_APIENTRY * glUniform2i)(GLint location, GLint x, GLint y);
void (GL_APIENTRY * glUniform2iv)(GLint location, GLsizei count, const GLint* v);
void (GL_APIENTRY * glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z);
void (GL_APIENTRY * glUniform3fv)(GLint location, GLsizei count, const GLfloat* v);
void (GL_APIENTRY * glUniform3i)(GLint location, GLint x, GLint y, GLint z);
void (GL_APIENTRY * glUniform3iv)(GLint location, GLsizei count, const GLint* v);
void (GL_APIENTRY * glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void (GL_APIENTRY * glUniform4fv)(GLint location, GLsizei count, const GLfloat* v);
void (GL_APIENTRY * glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w);
void (GL_APIENTRY * glUniform4iv)(GLint location, GLsizei count, const GLint* v);
void (GL_APIENTRY * glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void (GL_APIENTRY * glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void (GL_APIENTRY * glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void (GL_APIENTRY * glUseProgram)(GLuint program);
void (GL_APIENTRY * glValidateProgram)(GLuint program);
void (GL_APIENTRY * glVertexAttrib1f)(GLuint indx, GLfloat x);
void (GL_APIENTRY * glVertexAttrib1fv)(GLuint indx, const GLfloat* values);
void (GL_APIENTRY * glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y);
void (GL_APIENTRY * glVertexAttrib2fv)(GLuint indx, const GLfloat* values);
void (GL_APIENTRY * glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
void (GL_APIENTRY * glVertexAttrib3fv)(GLuint indx, const GLfloat* values);
void (GL_APIENTRY * glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void (GL_APIENTRY * glVertexAttrib4fv)(GLuint indx, const GLfloat* values);
void (GL_APIENTRY * glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
void (GL_APIENTRY * glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
#endif//SP2_GRAPHICS_OPENGLES2_H

namespace sp {

void initOpenGL()
{
    if (init_done)
        return;
    init_done = true;
    bool failure = false;

#ifdef SP2_GRAPHICS_OPENGLES2_H
    //OpenGL ES 2.0 functions
    if ((glActiveTexture = (decltype(glActiveTexture))SDL_GL_GetProcAddress("glActiveTexture")) == nullptr) { LOG(Error, "Failed to find opengl function: glActiveTexture"); failure = true; }
    if ((glAttachShader = (decltype(glAttachShader))SDL_GL_GetProcAddress("glAttachShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glAttachShader"); failure = true; }
    if ((glBindAttribLocation = (decltype(glBindAttribLocation))SDL_GL_GetProcAddress("glBindAttribLocation")) == nullptr) { LOG(Error, "Failed to find opengl function: glBindAttribLocation"); failure = true; }
    if ((glBindBuffer = (decltype(glBindBuffer))SDL_GL_GetProcAddress("glBindBuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glBindBuffer"); failure = true; }
    if ((glBindFramebuffer = (decltype(glBindFramebuffer))SDL_GL_GetProcAddress("glBindFramebuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glBindFramebuffer"); failure = true; }
    if ((glBindRenderbuffer = (decltype(glBindRenderbuffer))SDL_GL_GetProcAddress("glBindRenderbuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glBindRenderbuffer"); failure = true; }
    if ((glBindTexture = (decltype(glBindTexture))SDL_GL_GetProcAddress("glBindTexture")) == nullptr) { LOG(Error, "Failed to find opengl function: glBindTexture"); failure = true; }
    if ((glBlendColor = (decltype(glBlendColor))SDL_GL_GetProcAddress("glBlendColor")) == nullptr) { LOG(Error, "Failed to find opengl function: glBlendColor"); failure = true; }
    if ((glBlendEquation = (decltype(glBlendEquation))SDL_GL_GetProcAddress("glBlendEquation")) == nullptr) { LOG(Error, "Failed to find opengl function: glBlendEquation"); failure = true; }
    if ((glBlendEquationSeparate = (decltype(glBlendEquationSeparate))SDL_GL_GetProcAddress("glBlendEquationSeparate")) == nullptr) { LOG(Error, "Failed to find opengl function: glBlendEquationSeparate"); failure = true; }
    if ((glBlendFunc = (decltype(glBlendFunc))SDL_GL_GetProcAddress("glBlendFunc")) == nullptr) { LOG(Error, "Failed to find opengl function: glBlendFunc"); failure = true; }
    if ((glBlendFuncSeparate = (decltype(glBlendFuncSeparate))SDL_GL_GetProcAddress("glBlendFuncSeparate")) == nullptr) { LOG(Error, "Failed to find opengl function: glBlendFuncSeparate"); failure = true; }
    if ((glBufferData = (decltype(glBufferData))SDL_GL_GetProcAddress("glBufferData")) == nullptr) { LOG(Error, "Failed to find opengl function: glBufferData"); failure = true; }
    if ((glBufferSubData = (decltype(glBufferSubData))SDL_GL_GetProcAddress("glBufferSubData")) == nullptr) { LOG(Error, "Failed to find opengl function: glBufferSubData"); failure = true; }
    if ((glCheckFramebufferStatus = (decltype(glCheckFramebufferStatus))SDL_GL_GetProcAddress("glCheckFramebufferStatus")) == nullptr) { LOG(Error, "Failed to find opengl function: glCheckFramebufferStatus"); failure = true; }
    if ((glClear = (decltype(glClear))SDL_GL_GetProcAddress("glClear")) == nullptr) { LOG(Error, "Failed to find opengl function: glClear"); failure = true; }
    if ((glClearColor = (decltype(glClearColor))SDL_GL_GetProcAddress("glClearColor")) == nullptr) { LOG(Error, "Failed to find opengl function: glClearColor"); failure = true; }
    if ((glClearDepthf = (decltype(glClearDepthf))SDL_GL_GetProcAddress("glClearDepthf")) == nullptr) { LOG(Error, "Failed to find opengl function: glClearDepthf"); failure = true; }
    if ((glClearStencil = (decltype(glClearStencil))SDL_GL_GetProcAddress("glClearStencil")) == nullptr) { LOG(Error, "Failed to find opengl function: glClearStencil"); failure = true; }
    if ((glColorMask = (decltype(glColorMask))SDL_GL_GetProcAddress("glColorMask")) == nullptr) { LOG(Error, "Failed to find opengl function: glColorMask"); failure = true; }
    if ((glCompileShader = (decltype(glCompileShader))SDL_GL_GetProcAddress("glCompileShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glCompileShader"); failure = true; }
    if ((glCompressedTexImage2D = (decltype(glCompressedTexImage2D))SDL_GL_GetProcAddress("glCompressedTexImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glCompressedTexImage2D"); failure = true; }
    if ((glCompressedTexSubImage2D = (decltype(glCompressedTexSubImage2D))SDL_GL_GetProcAddress("glCompressedTexSubImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glCompressedTexSubImage2D"); failure = true; }
    if ((glCopyTexImage2D = (decltype(glCopyTexImage2D))SDL_GL_GetProcAddress("glCopyTexImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glCopyTexImage2D"); failure = true; }
    if ((glCopyTexSubImage2D = (decltype(glCopyTexSubImage2D))SDL_GL_GetProcAddress("glCopyTexSubImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glCopyTexSubImage2D"); failure = true; }
    if ((glCreateProgram = (decltype(glCreateProgram))SDL_GL_GetProcAddress("glCreateProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glCreateProgram"); failure = true; }
    if ((glCreateShader = (decltype(glCreateShader))SDL_GL_GetProcAddress("glCreateShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glCreateShader"); failure = true; }
    if ((glCullFace = (decltype(glCullFace))SDL_GL_GetProcAddress("glCullFace")) == nullptr) { LOG(Error, "Failed to find opengl function: glCullFace"); failure = true; }
    if ((glDeleteBuffers = (decltype(glDeleteBuffers))SDL_GL_GetProcAddress("glDeleteBuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteBuffers"); failure = true; }
    if ((glDeleteFramebuffers = (decltype(glDeleteFramebuffers))SDL_GL_GetProcAddress("glDeleteFramebuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteFramebuffers"); failure = true; }
    if ((glDeleteProgram = (decltype(glDeleteProgram))SDL_GL_GetProcAddress("glDeleteProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteProgram"); failure = true; }
    if ((glDeleteRenderbuffers = (decltype(glDeleteRenderbuffers))SDL_GL_GetProcAddress("glDeleteRenderbuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteRenderbuffers"); failure = true; }
    if ((glDeleteShader = (decltype(glDeleteShader))SDL_GL_GetProcAddress("glDeleteShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteShader"); failure = true; }
    if ((glDeleteTextures = (decltype(glDeleteTextures))SDL_GL_GetProcAddress("glDeleteTextures")) == nullptr) { LOG(Error, "Failed to find opengl function: glDeleteTextures"); failure = true; }
    if ((glDepthFunc = (decltype(glDepthFunc))SDL_GL_GetProcAddress("glDepthFunc")) == nullptr) { LOG(Error, "Failed to find opengl function: glDepthFunc"); failure = true; }
    if ((glDepthMask = (decltype(glDepthMask))SDL_GL_GetProcAddress("glDepthMask")) == nullptr) { LOG(Error, "Failed to find opengl function: glDepthMask"); failure = true; }
    if ((glDepthRangef = (decltype(glDepthRangef))SDL_GL_GetProcAddress("glDepthRangef")) == nullptr) { LOG(Error, "Failed to find opengl function: glDepthRangef"); failure = true; }
    if ((glDetachShader = (decltype(glDetachShader))SDL_GL_GetProcAddress("glDetachShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glDetachShader"); failure = true; }
    if ((glDisable = (decltype(glDisable))SDL_GL_GetProcAddress("glDisable")) == nullptr) { LOG(Error, "Failed to find opengl function: glDisable"); failure = true; }
    if ((glDisableVertexAttribArray = (decltype(glDisableVertexAttribArray))SDL_GL_GetProcAddress("glDisableVertexAttribArray")) == nullptr) { LOG(Error, "Failed to find opengl function: glDisableVertexAttribArray"); failure = true; }
    if ((glDrawArrays = (decltype(glDrawArrays))SDL_GL_GetProcAddress("glDrawArrays")) == nullptr) { LOG(Error, "Failed to find opengl function: glDrawArrays"); failure = true; }
    if ((glDrawElements = (decltype(glDrawElements))SDL_GL_GetProcAddress("glDrawElements")) == nullptr) { LOG(Error, "Failed to find opengl function: glDrawElements"); failure = true; }
    if ((glEnable = (decltype(glEnable))SDL_GL_GetProcAddress("glEnable")) == nullptr) { LOG(Error, "Failed to find opengl function: glEnable"); failure = true; }
    if ((glEnableVertexAttribArray = (decltype(glEnableVertexAttribArray))SDL_GL_GetProcAddress("glEnableVertexAttribArray")) == nullptr) { LOG(Error, "Failed to find opengl function: glEnableVertexAttribArray"); failure = true; }
    if ((glFinish = (decltype(glFinish))SDL_GL_GetProcAddress("glFinish")) == nullptr) { LOG(Error, "Failed to find opengl function: glFinish"); failure = true; }
    if ((glFlush = (decltype(glFlush))SDL_GL_GetProcAddress("glFlush")) == nullptr) { LOG(Error, "Failed to find opengl function: glFlush"); failure = true; }
    if ((glFramebufferRenderbuffer = (decltype(glFramebufferRenderbuffer))SDL_GL_GetProcAddress("glFramebufferRenderbuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glFramebufferRenderbuffer"); failure = true; }
    if ((glFramebufferTexture2D = (decltype(glFramebufferTexture2D))SDL_GL_GetProcAddress("glFramebufferTexture2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glFramebufferTexture2D"); failure = true; }
    if ((glFrontFace = (decltype(glFrontFace))SDL_GL_GetProcAddress("glFrontFace")) == nullptr) { LOG(Error, "Failed to find opengl function: glFrontFace"); failure = true; }
    if ((glGenBuffers = (decltype(glGenBuffers))SDL_GL_GetProcAddress("glGenBuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glGenBuffers"); failure = true; }
    if ((glGenerateMipmap = (decltype(glGenerateMipmap))SDL_GL_GetProcAddress("glGenerateMipmap")) == nullptr) { LOG(Error, "Failed to find opengl function: glGenerateMipmap"); failure = true; }
    if ((glGenFramebuffers = (decltype(glGenFramebuffers))SDL_GL_GetProcAddress("glGenFramebuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glGenFramebuffers"); failure = true; }
    if ((glGenRenderbuffers = (decltype(glGenRenderbuffers))SDL_GL_GetProcAddress("glGenRenderbuffers")) == nullptr) { LOG(Error, "Failed to find opengl function: glGenRenderbuffers"); failure = true; }
    if ((glGenTextures = (decltype(glGenTextures))SDL_GL_GetProcAddress("glGenTextures")) == nullptr) { LOG(Error, "Failed to find opengl function: glGenTextures"); failure = true; }
    if ((glGetActiveAttrib = (decltype(glGetActiveAttrib))SDL_GL_GetProcAddress("glGetActiveAttrib")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetActiveAttrib"); failure = true; }
    if ((glGetActiveUniform = (decltype(glGetActiveUniform))SDL_GL_GetProcAddress("glGetActiveUniform")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetActiveUniform"); failure = true; }
    if ((glGetAttachedShaders = (decltype(glGetAttachedShaders))SDL_GL_GetProcAddress("glGetAttachedShaders")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetAttachedShaders"); failure = true; }
    if ((glGetAttribLocation = (decltype(glGetAttribLocation))SDL_GL_GetProcAddress("glGetAttribLocation")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetAttribLocation"); failure = true; }
    if ((glGetBooleanv = (decltype(glGetBooleanv))SDL_GL_GetProcAddress("glGetBooleanv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetBooleanv"); failure = true; }
    if ((glGetBufferParameteriv = (decltype(glGetBufferParameteriv))SDL_GL_GetProcAddress("glGetBufferParameteriv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetBufferParameteriv"); failure = true; }
    if ((glGetError = (decltype(glGetError))SDL_GL_GetProcAddress("glGetError")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetError"); failure = true; }
    if ((glGetFloatv = (decltype(glGetFloatv))SDL_GL_GetProcAddress("glGetFloatv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetFloatv"); failure = true; }
    if ((glGetFramebufferAttachmentParameteriv = (decltype(glGetFramebufferAttachmentParameteriv))SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameteriv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetFramebufferAttachmentParameteriv"); failure = true; }
    if ((glGetIntegerv = (decltype(glGetIntegerv))SDL_GL_GetProcAddress("glGetIntegerv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetIntegerv"); failure = true; }
    if ((glGetProgramiv = (decltype(glGetProgramiv))SDL_GL_GetProcAddress("glGetProgramiv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetProgramiv"); failure = true; }
    if ((glGetProgramInfoLog = (decltype(glGetProgramInfoLog))SDL_GL_GetProcAddress("glGetProgramInfoLog")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetProgramInfoLog"); failure = true; }
    if ((glGetRenderbufferParameteriv = (decltype(glGetRenderbufferParameteriv))SDL_GL_GetProcAddress("glGetRenderbufferParameteriv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetRenderbufferParameteriv"); failure = true; }
    if ((glGetShaderiv = (decltype(glGetShaderiv))SDL_GL_GetProcAddress("glGetShaderiv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetShaderiv"); failure = true; }
    if ((glGetShaderInfoLog = (decltype(glGetShaderInfoLog))SDL_GL_GetProcAddress("glGetShaderInfoLog")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetShaderInfoLog"); failure = true; }
    if ((glGetShaderPrecisionFormat = (decltype(glGetShaderPrecisionFormat))SDL_GL_GetProcAddress("glGetShaderPrecisionFormat")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetShaderPrecisionFormat"); failure = true; }
    if ((glGetShaderSource = (decltype(glGetShaderSource))SDL_GL_GetProcAddress("glGetShaderSource")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetShaderSource"); failure = true; }
    if ((glGetString = (decltype(glGetString))SDL_GL_GetProcAddress("glGetString")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetString"); failure = true; }
    if ((glGetTexParameterfv = (decltype(glGetTexParameterfv))SDL_GL_GetProcAddress("glGetTexParameterfv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetTexParameterfv"); failure = true; }
    if ((glGetTexParameteriv = (decltype(glGetTexParameteriv))SDL_GL_GetProcAddress("glGetTexParameteriv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetTexParameteriv"); failure = true; }
    if ((glGetUniformfv = (decltype(glGetUniformfv))SDL_GL_GetProcAddress("glGetUniformfv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetUniformfv"); failure = true; }
    if ((glGetUniformiv = (decltype(glGetUniformiv))SDL_GL_GetProcAddress("glGetUniformiv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetUniformiv"); failure = true; }
    if ((glGetUniformLocation = (decltype(glGetUniformLocation))SDL_GL_GetProcAddress("glGetUniformLocation")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetUniformLocation"); failure = true; }
    if ((glGetVertexAttribfv = (decltype(glGetVertexAttribfv))SDL_GL_GetProcAddress("glGetVertexAttribfv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetVertexAttribfv"); failure = true; }
    if ((glGetVertexAttribiv = (decltype(glGetVertexAttribiv))SDL_GL_GetProcAddress("glGetVertexAttribiv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetVertexAttribiv"); failure = true; }
    if ((glGetVertexAttribPointerv = (decltype(glGetVertexAttribPointerv))SDL_GL_GetProcAddress("glGetVertexAttribPointerv")) == nullptr) { LOG(Error, "Failed to find opengl function: glGetVertexAttribPointerv"); failure = true; }
    if ((glHint = (decltype(glHint))SDL_GL_GetProcAddress("glHint")) == nullptr) { LOG(Error, "Failed to find opengl function: glHint"); failure = true; }
    if ((glIsBuffer = (decltype(glIsBuffer))SDL_GL_GetProcAddress("glIsBuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsBuffer"); failure = true; }
    if ((glIsEnabled = (decltype(glIsEnabled))SDL_GL_GetProcAddress("glIsEnabled")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsEnabled"); failure = true; }
    if ((glIsFramebuffer = (decltype(glIsFramebuffer))SDL_GL_GetProcAddress("glIsFramebuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsFramebuffer"); failure = true; }
    if ((glIsProgram = (decltype(glIsProgram))SDL_GL_GetProcAddress("glIsProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsProgram"); failure = true; }
    if ((glIsRenderbuffer = (decltype(glIsRenderbuffer))SDL_GL_GetProcAddress("glIsRenderbuffer")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsRenderbuffer"); failure = true; }
    if ((glIsShader = (decltype(glIsShader))SDL_GL_GetProcAddress("glIsShader")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsShader"); failure = true; }
    if ((glIsTexture = (decltype(glIsTexture))SDL_GL_GetProcAddress("glIsTexture")) == nullptr) { LOG(Error, "Failed to find opengl function: glIsTexture"); failure = true; }
    if ((glLineWidth = (decltype(glLineWidth))SDL_GL_GetProcAddress("glLineWidth")) == nullptr) { LOG(Error, "Failed to find opengl function: glLineWidth"); failure = true; }
    if ((glLinkProgram = (decltype(glLinkProgram))SDL_GL_GetProcAddress("glLinkProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glLinkProgram"); failure = true; }
    if ((glPixelStorei = (decltype(glPixelStorei))SDL_GL_GetProcAddress("glPixelStorei")) == nullptr) { LOG(Error, "Failed to find opengl function: glPixelStorei"); failure = true; }
    if ((glPolygonOffset = (decltype(glPolygonOffset))SDL_GL_GetProcAddress("glPolygonOffset")) == nullptr) { LOG(Error, "Failed to find opengl function: glPolygonOffset"); failure = true; }
    if ((glReadPixels = (decltype(glReadPixels))SDL_GL_GetProcAddress("glReadPixels")) == nullptr) { LOG(Error, "Failed to find opengl function: glReadPixels"); failure = true; }
    if ((glReleaseShaderCompiler = (decltype(glReleaseShaderCompiler))SDL_GL_GetProcAddress("glReleaseShaderCompiler")) == nullptr) { LOG(Error, "Failed to find opengl function: glReleaseShaderCompiler"); failure = true; }
    if ((glRenderbufferStorage = (decltype(glRenderbufferStorage))SDL_GL_GetProcAddress("glRenderbufferStorage")) == nullptr) { LOG(Error, "Failed to find opengl function: glRenderbufferStorage"); failure = true; }
    if ((glSampleCoverage = (decltype(glSampleCoverage))SDL_GL_GetProcAddress("glSampleCoverage")) == nullptr) { LOG(Error, "Failed to find opengl function: glSampleCoverage"); failure = true; }
    if ((glScissor = (decltype(glScissor))SDL_GL_GetProcAddress("glScissor")) == nullptr) { LOG(Error, "Failed to find opengl function: glScissor"); failure = true; }
    if ((glShaderBinary = (decltype(glShaderBinary))SDL_GL_GetProcAddress("glShaderBinary")) == nullptr) { LOG(Error, "Failed to find opengl function: glShaderBinary"); failure = true; }
    if ((glShaderSource = (decltype(glShaderSource))SDL_GL_GetProcAddress("glShaderSource")) == nullptr) { LOG(Error, "Failed to find opengl function: glShaderSource"); failure = true; }
    if ((glStencilFunc = (decltype(glStencilFunc))SDL_GL_GetProcAddress("glStencilFunc")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilFunc"); failure = true; }
    if ((glStencilFuncSeparate = (decltype(glStencilFuncSeparate))SDL_GL_GetProcAddress("glStencilFuncSeparate")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilFuncSeparate"); failure = true; }
    if ((glStencilMask = (decltype(glStencilMask))SDL_GL_GetProcAddress("glStencilMask")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilMask"); failure = true; }
    if ((glStencilMaskSeparate = (decltype(glStencilMaskSeparate))SDL_GL_GetProcAddress("glStencilMaskSeparate")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilMaskSeparate"); failure = true; }
    if ((glStencilOp = (decltype(glStencilOp))SDL_GL_GetProcAddress("glStencilOp")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilOp"); failure = true; }
    if ((glStencilOpSeparate = (decltype(glStencilOpSeparate))SDL_GL_GetProcAddress("glStencilOpSeparate")) == nullptr) { LOG(Error, "Failed to find opengl function: glStencilOpSeparate"); failure = true; }
    if ((glTexImage2D = (decltype(glTexImage2D))SDL_GL_GetProcAddress("glTexImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexImage2D"); failure = true; }
    if ((glTexParameterf = (decltype(glTexParameterf))SDL_GL_GetProcAddress("glTexParameterf")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexParameterf"); failure = true; }
    if ((glTexParameterfv = (decltype(glTexParameterfv))SDL_GL_GetProcAddress("glTexParameterfv")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexParameterfv"); failure = true; }
    if ((glTexParameteri = (decltype(glTexParameteri))SDL_GL_GetProcAddress("glTexParameteri")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexParameteri"); failure = true; }
    if ((glTexParameteriv = (decltype(glTexParameteriv))SDL_GL_GetProcAddress("glTexParameteriv")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexParameteriv"); failure = true; }
    if ((glTexSubImage2D = (decltype(glTexSubImage2D))SDL_GL_GetProcAddress("glTexSubImage2D")) == nullptr) { LOG(Error, "Failed to find opengl function: glTexSubImage2D"); failure = true; }
    if ((glUniform1f = (decltype(glUniform1f))SDL_GL_GetProcAddress("glUniform1f")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform1f"); failure = true; }
    if ((glUniform1fv = (decltype(glUniform1fv))SDL_GL_GetProcAddress("glUniform1fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform1fv"); failure = true; }
    if ((glUniform1i = (decltype(glUniform1i))SDL_GL_GetProcAddress("glUniform1i")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform1i"); failure = true; }
    if ((glUniform1iv = (decltype(glUniform1iv))SDL_GL_GetProcAddress("glUniform1iv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform1iv"); failure = true; }
    if ((glUniform2f = (decltype(glUniform2f))SDL_GL_GetProcAddress("glUniform2f")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform2f"); failure = true; }
    if ((glUniform2fv = (decltype(glUniform2fv))SDL_GL_GetProcAddress("glUniform2fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform2fv"); failure = true; }
    if ((glUniform2i = (decltype(glUniform2i))SDL_GL_GetProcAddress("glUniform2i")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform2i"); failure = true; }
    if ((glUniform2iv = (decltype(glUniform2iv))SDL_GL_GetProcAddress("glUniform2iv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform2iv"); failure = true; }
    if ((glUniform3f = (decltype(glUniform3f))SDL_GL_GetProcAddress("glUniform3f")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform3f"); failure = true; }
    if ((glUniform3fv = (decltype(glUniform3fv))SDL_GL_GetProcAddress("glUniform3fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform3fv"); failure = true; }
    if ((glUniform3i = (decltype(glUniform3i))SDL_GL_GetProcAddress("glUniform3i")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform3i"); failure = true; }
    if ((glUniform3iv = (decltype(glUniform3iv))SDL_GL_GetProcAddress("glUniform3iv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform3iv"); failure = true; }
    if ((glUniform4f = (decltype(glUniform4f))SDL_GL_GetProcAddress("glUniform4f")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform4f"); failure = true; }
    if ((glUniform4fv = (decltype(glUniform4fv))SDL_GL_GetProcAddress("glUniform4fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform4fv"); failure = true; }
    if ((glUniform4i = (decltype(glUniform4i))SDL_GL_GetProcAddress("glUniform4i")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform4i"); failure = true; }
    if ((glUniform4iv = (decltype(glUniform4iv))SDL_GL_GetProcAddress("glUniform4iv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniform4iv"); failure = true; }
    if ((glUniformMatrix2fv = (decltype(glUniformMatrix2fv))SDL_GL_GetProcAddress("glUniformMatrix2fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniformMatrix2fv"); failure = true; }
    if ((glUniformMatrix3fv = (decltype(glUniformMatrix3fv))SDL_GL_GetProcAddress("glUniformMatrix3fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniformMatrix3fv"); failure = true; }
    if ((glUniformMatrix4fv = (decltype(glUniformMatrix4fv))SDL_GL_GetProcAddress("glUniformMatrix4fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glUniformMatrix4fv"); failure = true; }
    if ((glUseProgram = (decltype(glUseProgram))SDL_GL_GetProcAddress("glUseProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glUseProgram"); failure = true; }
    if ((glValidateProgram = (decltype(glValidateProgram))SDL_GL_GetProcAddress("glValidateProgram")) == nullptr) { LOG(Error, "Failed to find opengl function: glValidateProgram"); failure = true; }
    if ((glVertexAttrib1f = (decltype(glVertexAttrib1f))SDL_GL_GetProcAddress("glVertexAttrib1f")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib1f"); failure = true; }
    if ((glVertexAttrib1fv = (decltype(glVertexAttrib1fv))SDL_GL_GetProcAddress("glVertexAttrib1fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib1fv"); failure = true; }
    if ((glVertexAttrib2f = (decltype(glVertexAttrib2f))SDL_GL_GetProcAddress("glVertexAttrib2f")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib2f"); failure = true; }
    if ((glVertexAttrib2fv = (decltype(glVertexAttrib2fv))SDL_GL_GetProcAddress("glVertexAttrib2fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib2fv"); failure = true; }
    if ((glVertexAttrib3f = (decltype(glVertexAttrib3f))SDL_GL_GetProcAddress("glVertexAttrib3f")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib3f"); failure = true; }
    if ((glVertexAttrib3fv = (decltype(glVertexAttrib3fv))SDL_GL_GetProcAddress("glVertexAttrib3fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib3fv"); failure = true; }
    if ((glVertexAttrib4f = (decltype(glVertexAttrib4f))SDL_GL_GetProcAddress("glVertexAttrib4f")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib4f"); failure = true; }
    if ((glVertexAttrib4fv = (decltype(glVertexAttrib4fv))SDL_GL_GetProcAddress("glVertexAttrib4fv")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttrib4fv"); failure = true; }
    if ((glVertexAttribPointer = (decltype(glVertexAttribPointer))SDL_GL_GetProcAddress("glVertexAttribPointer")) == nullptr) { LOG(Error, "Failed to find opengl function: glVertexAttribPointer"); failure = true; }
    if ((glViewport = (decltype(glViewport))SDL_GL_GetProcAddress("glViewport")) == nullptr) { LOG(Error, "Failed to find opengl function: glViewport"); failure = true; }
#endif//SP2_GRAPHICS_OPENGLES2_H

    if (failure)
        exit(1);
}

};//namespace sp


#ifdef SP2_ENABLE_OPENGL_TRACING
namespace sp {
void traceOpenGLCall(const char* function_name, const char* source_file, const char* source_function, int source_line_number, sp::string parameters)
{
    int error = glGetError();
#ifdef ANDROID
    LOG(Debug, "GL_TRACE", source_file, source_line_number, source_function, function_name, parameters);
    if (error)
        LOG(Error, "GL_TRACE ERROR", error);
#else
    static FILE* f = nullptr;
    if (!f)
        f = fopen("opengl.trace.txt", "wt");
    fprintf(f, "%80s:%4d %60s %s %s\n", source_file, source_line_number, source_function, function_name, parameters.c_str());
    if (error)
        fprintf(f, "ERROR: %d\n", error);
#endif
}
};//namespace sp
#endif//SP2_ENABLE_OPENGL_TRACING
