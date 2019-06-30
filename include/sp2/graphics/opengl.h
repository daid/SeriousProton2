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


//#define SP2_ENABLE_OPENGL_TRACING
#ifdef SP2_ENABLE_OPENGL_TRACING
#include <sp2/string.h>
namespace sp {
void traceOpenGLCall(const char* function_name, const char* source_file, const char* source_function, int source_line_number, sp::string parameters);
static inline sp::string traceOpenGLCallParams() { return ""; }
static inline sp::string traceOpenGLCallParam(int n) { return sp::string(n); }
static inline sp::string traceOpenGLCallParam(const void* ptr) { return "[ptr]"; }
template<typename A1> sp::string traceOpenGLCallParams(const A1& a) { return traceOpenGLCallParam(a); }
template<typename A1, typename... ARGS> sp::string traceOpenGLCallParams(const A1& a, const ARGS&... args) { return traceOpenGLCallParam(a) + ", " + traceOpenGLCallParams(args...); }
};//namespace sp

#define glActiveTexture(...) do { glActiveTexture(__VA_ARGS__); traceOpenGLCall("glActiveTexture", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glAttachShader(...) do { glAttachShader(__VA_ARGS__); traceOpenGLCall("glAttachShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBindAttribLocation(...) do { glBindAttribLocation(__VA_ARGS__); traceOpenGLCall("glBindAttribLocation", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBindBuffer(...) do { glBindBuffer(__VA_ARGS__); traceOpenGLCall("glBindBuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBindFramebuffer(...) do { glBindFramebuffer(__VA_ARGS__); traceOpenGLCall("glBindFramebuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBindRenderbuffer(...) do { glBindRenderbuffer(__VA_ARGS__); traceOpenGLCall("glBindRenderbuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBindTexture(...) do { glBindTexture(__VA_ARGS__); traceOpenGLCall("glBindTexture", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBlendColor(...) do { glBlendColor(__VA_ARGS__); traceOpenGLCall("glBlendColor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBlendEquation(...) do { glBlendEquation(__VA_ARGS__); traceOpenGLCall("glBlendEquation", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBlendEquationSeparate(...) do { glBlendEquationSeparate(__VA_ARGS__); traceOpenGLCall("glBlendEquationSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBlendFunc(...) do { glBlendFunc(__VA_ARGS__); traceOpenGLCall("glBlendFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBlendFuncSeparate(...) do { glBlendFuncSeparate(__VA_ARGS__); traceOpenGLCall("glBlendFuncSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBufferData(...) do { glBufferData(__VA_ARGS__); traceOpenGLCall("glBufferData", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glBufferSubData(...) do { glBufferSubData(__VA_ARGS__); traceOpenGLCall("glBufferSubData", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glClear(...) do { glClear(__VA_ARGS__); traceOpenGLCall("glClear", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glClearColor(...) do { glClearColor(__VA_ARGS__); traceOpenGLCall("glClearColor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glClearDepthf(...) do { glClearDepthf(__VA_ARGS__); traceOpenGLCall("glClearDepthf", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glClearStencil(...) do { glClearStencil(__VA_ARGS__); traceOpenGLCall("glClearStencil", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glColorMask(...) do { glColorMask(__VA_ARGS__); traceOpenGLCall("glColorMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCompileShader(...) do { glCompileShader(__VA_ARGS__); traceOpenGLCall("glCompileShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCompressedTexImage2D(...) do { glCompressedTexImage2D(__VA_ARGS__); traceOpenGLCall("glCompressedTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCompressedTexSubImage2D(...) do { glCompressedTexSubImage2D(__VA_ARGS__); traceOpenGLCall("glCompressedTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCopyTexImage2D(...) do { glCopyTexImage2D(__VA_ARGS__); traceOpenGLCall("glCopyTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCopyTexSubImage2D(...) do { glCopyTexSubImage2D(__VA_ARGS__); traceOpenGLCall("glCopyTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glCullFace(...) do { glCullFace(__VA_ARGS__); traceOpenGLCall("glCullFace", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteBuffers(...) do { glDeleteBuffers(__VA_ARGS__); traceOpenGLCall("glDeleteBuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteFramebuffers(...) do { glDeleteFramebuffers(__VA_ARGS__); traceOpenGLCall("glDeleteFramebuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteProgram(...) do { glDeleteProgram(__VA_ARGS__); traceOpenGLCall("glDeleteProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteRenderbuffers(...) do { glDeleteRenderbuffers(__VA_ARGS__); traceOpenGLCall("glDeleteRenderbuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteShader(...) do { glDeleteShader(__VA_ARGS__); traceOpenGLCall("glDeleteShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDeleteTextures(...) do { glDeleteTextures(__VA_ARGS__); traceOpenGLCall("glDeleteTextures", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDepthFunc(...) do { glDepthFunc(__VA_ARGS__); traceOpenGLCall("glDepthFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDepthMask(...) do { glDepthMask(__VA_ARGS__); traceOpenGLCall("glDepthMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDepthRangef(...) do { glDepthRangef(__VA_ARGS__); traceOpenGLCall("glDepthRangef", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDetachShader(...) do { glDetachShader(__VA_ARGS__); traceOpenGLCall("glDetachShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDisable(...) do { glDisable(__VA_ARGS__); traceOpenGLCall("glDisable", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDisableVertexAttribArray(...) do { glDisableVertexAttribArray(__VA_ARGS__); traceOpenGLCall("glDisableVertexAttribArray", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDrawArrays(...) do { glDrawArrays(__VA_ARGS__); traceOpenGLCall("glDrawArrays", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glDrawElements(...) do { glDrawElements(__VA_ARGS__); traceOpenGLCall("glDrawElements", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glEnable(...) do { glEnable(__VA_ARGS__); traceOpenGLCall("glEnable", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glEnableVertexAttribArray(...) do { glEnableVertexAttribArray(__VA_ARGS__); traceOpenGLCall("glEnableVertexAttribArray", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glFinish(...) do { glFinish(__VA_ARGS__); traceOpenGLCall("glFinish", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glFlush(...) do { glFlush(__VA_ARGS__); traceOpenGLCall("glFlush", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glFramebufferRenderbuffer(...) do { glFramebufferRenderbuffer(__VA_ARGS__); traceOpenGLCall("glFramebufferRenderbuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glFramebufferTexture2D(...) do { glFramebufferTexture2D(__VA_ARGS__); traceOpenGLCall("glFramebufferTexture2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glFrontFace(...) do { glFrontFace(__VA_ARGS__); traceOpenGLCall("glFrontFace", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGenBuffers(...) do { glGenBuffers(__VA_ARGS__); traceOpenGLCall("glGenBuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGenerateMipmap(...) do { glGenerateMipmap(__VA_ARGS__); traceOpenGLCall("glGenerateMipmap", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGenFramebuffers(...) do { glGenFramebuffers(__VA_ARGS__); traceOpenGLCall("glGenFramebuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGenRenderbuffers(...) do { glGenRenderbuffers(__VA_ARGS__); traceOpenGLCall("glGenRenderbuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGenTextures(...) do { glGenTextures(__VA_ARGS__); traceOpenGLCall("glGenTextures", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetActiveAttrib(...) do { glGetActiveAttrib(__VA_ARGS__); traceOpenGLCall("glGetActiveAttrib", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetActiveUniform(...) do { glGetActiveUniform(__VA_ARGS__); traceOpenGLCall("glGetActiveUniform", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetAttachedShaders(...) do { glGetAttachedShaders(__VA_ARGS__); traceOpenGLCall("glGetAttachedShaders", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetBooleanv(...) do { glGetBooleanv(__VA_ARGS__); traceOpenGLCall("glGetBooleanv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetBufferParameteriv(...) do { glGetBufferParameteriv(__VA_ARGS__); traceOpenGLCall("glGetBufferParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetFloatv(...) do { glGetFloatv(__VA_ARGS__); traceOpenGLCall("glGetFloatv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetFramebufferAttachmentParameteriv(...) do { glGetFramebufferAttachmentParameteriv(__VA_ARGS__); traceOpenGLCall("glGetFramebufferAttachmentParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetIntegerv(...) do { glGetIntegerv(__VA_ARGS__); traceOpenGLCall("glGetIntegerv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetProgramiv(...) do { glGetProgramiv(__VA_ARGS__); traceOpenGLCall("glGetProgramiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetProgramInfoLog(...) do { glGetProgramInfoLog(__VA_ARGS__); traceOpenGLCall("glGetProgramInfoLog", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetRenderbufferParameteriv(...) do { glGetRenderbufferParameteriv(__VA_ARGS__); traceOpenGLCall("glGetRenderbufferParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetShaderiv(...) do { glGetShaderiv(__VA_ARGS__); traceOpenGLCall("glGetShaderiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetShaderInfoLog(...) do { glGetShaderInfoLog(__VA_ARGS__); traceOpenGLCall("glGetShaderInfoLog", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetShaderPrecisionFormat(...) do { glGetShaderPrecisionFormat(__VA_ARGS__); traceOpenGLCall("glGetShaderPrecisionFormat", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetShaderSource(...) do { glGetShaderSource(__VA_ARGS__); traceOpenGLCall("glGetShaderSource", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetTexParameterfv(...) do { glGetTexParameterfv(__VA_ARGS__); traceOpenGLCall("glGetTexParameterfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetTexParameteriv(...) do { glGetTexParameteriv(__VA_ARGS__); traceOpenGLCall("glGetTexParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetUniformfv(...) do { glGetUniformfv(__VA_ARGS__); traceOpenGLCall("glGetUniformfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetUniformiv(...) do { glGetUniformiv(__VA_ARGS__); traceOpenGLCall("glGetUniformiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetVertexAttribfv(...) do { glGetVertexAttribfv(__VA_ARGS__); traceOpenGLCall("glGetVertexAttribfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetVertexAttribiv(...) do { glGetVertexAttribiv(__VA_ARGS__); traceOpenGLCall("glGetVertexAttribiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glGetVertexAttribPointerv(...) do { glGetVertexAttribPointerv(__VA_ARGS__); traceOpenGLCall("glGetVertexAttribPointerv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glHint(...) do { glHint(__VA_ARGS__); traceOpenGLCall("glHint", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glLineWidth(...) do { glLineWidth(__VA_ARGS__); traceOpenGLCall("glLineWidth", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glLinkProgram(...) do { glLinkProgram(__VA_ARGS__); traceOpenGLCall("glLinkProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glPixelStorei(...) do { glPixelStorei(__VA_ARGS__); traceOpenGLCall("glPixelStorei", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glPolygonOffset(...) do { glPolygonOffset(__VA_ARGS__); traceOpenGLCall("glPolygonOffset", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glReadPixels(...) do { glReadPixels(__VA_ARGS__); traceOpenGLCall("glReadPixels", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glReleaseShaderCompiler(...) do { glReleaseShaderCompiler(__VA_ARGS__); traceOpenGLCall("glReleaseShaderCompiler", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glRenderbufferStorage(...) do { glRenderbufferStorage(__VA_ARGS__); traceOpenGLCall("glRenderbufferStorage", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glSampleCoverage(...) do { glSampleCoverage(__VA_ARGS__); traceOpenGLCall("glSampleCoverage", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glScissor(...) do { glScissor(__VA_ARGS__); traceOpenGLCall("glScissor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glShaderBinary(...) do { glShaderBinary(__VA_ARGS__); traceOpenGLCall("glShaderBinary", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glShaderSource(...) do { glShaderSource(__VA_ARGS__); traceOpenGLCall("glShaderSource", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilFunc(...) do { glStencilFunc(__VA_ARGS__); traceOpenGLCall("glStencilFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilFuncSeparate(...) do { glStencilFuncSeparate(__VA_ARGS__); traceOpenGLCall("glStencilFuncSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilMask(...) do { glStencilMask(__VA_ARGS__); traceOpenGLCall("glStencilMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilMaskSeparate(...) do { glStencilMaskSeparate(__VA_ARGS__); traceOpenGLCall("glStencilMaskSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilOp(...) do { glStencilOp(__VA_ARGS__); traceOpenGLCall("glStencilOp", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glStencilOpSeparate(...) do { glStencilOpSeparate(__VA_ARGS__); traceOpenGLCall("glStencilOpSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexImage2D(...) do { glTexImage2D(__VA_ARGS__); traceOpenGLCall("glTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexParameterf(...) do { glTexParameterf(__VA_ARGS__); traceOpenGLCall("glTexParameterf", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexParameterfv(...) do { glTexParameterfv(__VA_ARGS__); traceOpenGLCall("glTexParameterfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexParameteri(...) do { glTexParameteri(__VA_ARGS__); traceOpenGLCall("glTexParameteri", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexParameteriv(...) do { glTexParameteriv(__VA_ARGS__); traceOpenGLCall("glTexParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glTexSubImage2D(...) do { glTexSubImage2D(__VA_ARGS__); traceOpenGLCall("glTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform1f(...) do { glUniform1f(__VA_ARGS__); traceOpenGLCall("glUniform1f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform1fv(...) do { glUniform1fv(__VA_ARGS__); traceOpenGLCall("glUniform1fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform1i(...) do { glUniform1i(__VA_ARGS__); traceOpenGLCall("glUniform1i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform1iv(...) do { glUniform1iv(__VA_ARGS__); traceOpenGLCall("glUniform1iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform2f(...) do { glUniform2f(__VA_ARGS__); traceOpenGLCall("glUniform2f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform2fv(...) do { glUniform2fv(__VA_ARGS__); traceOpenGLCall("glUniform2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform2i(...) do { glUniform2i(__VA_ARGS__); traceOpenGLCall("glUniform2i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform2iv(...) do { glUniform2iv(__VA_ARGS__); traceOpenGLCall("glUniform2iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform3f(...) do { glUniform3f(__VA_ARGS__); traceOpenGLCall("glUniform3f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform3fv(...) do { glUniform3fv(__VA_ARGS__); traceOpenGLCall("glUniform3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform3i(...) do { glUniform3i(__VA_ARGS__); traceOpenGLCall("glUniform3i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform3iv(...) do { glUniform3iv(__VA_ARGS__); traceOpenGLCall("glUniform3iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform4f(...) do { glUniform4f(__VA_ARGS__); traceOpenGLCall("glUniform4f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform4fv(...) do { glUniform4fv(__VA_ARGS__); traceOpenGLCall("glUniform4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform4i(...) do { glUniform4i(__VA_ARGS__); traceOpenGLCall("glUniform4i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniform4iv(...) do { glUniform4iv(__VA_ARGS__); traceOpenGLCall("glUniform4iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniformMatrix2fv(...) do { glUniformMatrix2fv(__VA_ARGS__); traceOpenGLCall("glUniformMatrix2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniformMatrix3fv(...) do { glUniformMatrix3fv(__VA_ARGS__); traceOpenGLCall("glUniformMatrix3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUniformMatrix4fv(...) do { glUniformMatrix4fv(__VA_ARGS__); traceOpenGLCall("glUniformMatrix4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glUseProgram(...) do { glUseProgram(__VA_ARGS__); traceOpenGLCall("glUseProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glValidateProgram(...) do { glValidateProgram(__VA_ARGS__); traceOpenGLCall("glValidateProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib1f(...) do { glVertexAttrib1f(__VA_ARGS__); traceOpenGLCall("glVertexAttrib1f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib1fv(...) do { glVertexAttrib1fv(__VA_ARGS__); traceOpenGLCall("glVertexAttrib1fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib2f(...) do { glVertexAttrib2f(__VA_ARGS__); traceOpenGLCall("glVertexAttrib2f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib2fv(...) do { glVertexAttrib2fv(__VA_ARGS__); traceOpenGLCall("glVertexAttrib2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib3f(...) do { glVertexAttrib3f(__VA_ARGS__); traceOpenGLCall("glVertexAttrib3f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib3fv(...) do { glVertexAttrib3fv(__VA_ARGS__); traceOpenGLCall("glVertexAttrib3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib4f(...) do { glVertexAttrib4f(__VA_ARGS__); traceOpenGLCall("glVertexAttrib4f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttrib4fv(...) do { glVertexAttrib4fv(__VA_ARGS__); traceOpenGLCall("glVertexAttrib4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glVertexAttribPointer(...) do { glVertexAttribPointer(__VA_ARGS__); traceOpenGLCall("glVertexAttribPointer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)
#define glViewport(...) do { glViewport(__VA_ARGS__); traceOpenGLCall("glViewport", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); } while(0)

#endif//SP2_ENABLE_OPENGL_TRACING

#endif//SP2_GRAPHICS_OPENGL_H
