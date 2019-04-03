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

#define glActiveTexture(...) do { traceOpenGLCall("glActiveTexture", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glActiveTexture(__VA_ARGS__); } while(0)
#define glAttachShader(...) do { traceOpenGLCall("glAttachShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glAttachShader(__VA_ARGS__); } while(0)
#define glBindAttribLocation(...) do { traceOpenGLCall("glBindAttribLocation", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBindAttribLocation(__VA_ARGS__); } while(0)
#define glBindBuffer(...) do { traceOpenGLCall("glBindBuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBindBuffer(__VA_ARGS__); } while(0)
#define glBindFramebuffer(...) do { traceOpenGLCall("glBindFramebuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBindFramebuffer(__VA_ARGS__); } while(0)
#define glBindRenderbuffer(...) do { traceOpenGLCall("glBindRenderbuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBindRenderbuffer(__VA_ARGS__); } while(0)
#define glBindTexture(...) do { traceOpenGLCall("glBindTexture", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBindTexture(__VA_ARGS__); } while(0)
#define glBlendColor(...) do { traceOpenGLCall("glBlendColor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBlendColor(__VA_ARGS__); } while(0)
#define glBlendEquation(...) do { traceOpenGLCall("glBlendEquation", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBlendEquation(__VA_ARGS__); } while(0)
#define glBlendEquationSeparate(...) do { traceOpenGLCall("glBlendEquationSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBlendEquationSeparate(__VA_ARGS__); } while(0)
#define glBlendFunc(...) do { traceOpenGLCall("glBlendFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBlendFunc(__VA_ARGS__); } while(0)
#define glBlendFuncSeparate(...) do { traceOpenGLCall("glBlendFuncSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBlendFuncSeparate(__VA_ARGS__); } while(0)
#define glBufferData(...) do { traceOpenGLCall("glBufferData", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBufferData(__VA_ARGS__); } while(0)
#define glBufferSubData(...) do { traceOpenGLCall("glBufferSubData", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glBufferSubData(__VA_ARGS__); } while(0)
#define glClear(...) do { traceOpenGLCall("glClear", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glClear(__VA_ARGS__); } while(0)
#define glClearColor(...) do { traceOpenGLCall("glClearColor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glClearColor(__VA_ARGS__); } while(0)
#define glClearDepthf(...) do { traceOpenGLCall("glClearDepthf", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glClearDepthf(__VA_ARGS__); } while(0)
#define glClearStencil(...) do { traceOpenGLCall("glClearStencil", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glClearStencil(__VA_ARGS__); } while(0)
#define glColorMask(...) do { traceOpenGLCall("glColorMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glColorMask(__VA_ARGS__); } while(0)
#define glCompileShader(...) do { traceOpenGLCall("glCompileShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCompileShader(__VA_ARGS__); } while(0)
#define glCompressedTexImage2D(...) do { traceOpenGLCall("glCompressedTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCompressedTexImage2D(__VA_ARGS__); } while(0)
#define glCompressedTexSubImage2D(...) do { traceOpenGLCall("glCompressedTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCompressedTexSubImage2D(__VA_ARGS__); } while(0)
#define glCopyTexImage2D(...) do { traceOpenGLCall("glCopyTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCopyTexImage2D(__VA_ARGS__); } while(0)
#define glCopyTexSubImage2D(...) do { traceOpenGLCall("glCopyTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCopyTexSubImage2D(__VA_ARGS__); } while(0)
#define glCullFace(...) do { traceOpenGLCall("glCullFace", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glCullFace(__VA_ARGS__); } while(0)
#define glDeleteBuffers(...) do { traceOpenGLCall("glDeleteBuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteBuffers(__VA_ARGS__); } while(0)
#define glDeleteFramebuffers(...) do { traceOpenGLCall("glDeleteFramebuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteFramebuffers(__VA_ARGS__); } while(0)
#define glDeleteProgram(...) do { traceOpenGLCall("glDeleteProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteProgram(__VA_ARGS__); } while(0)
#define glDeleteRenderbuffers(...) do { traceOpenGLCall("glDeleteRenderbuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteRenderbuffers(__VA_ARGS__); } while(0)
#define glDeleteShader(...) do { traceOpenGLCall("glDeleteShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteShader(__VA_ARGS__); } while(0)
#define glDeleteTextures(...) do { traceOpenGLCall("glDeleteTextures", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDeleteTextures(__VA_ARGS__); } while(0)
#define glDepthFunc(...) do { traceOpenGLCall("glDepthFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDepthFunc(__VA_ARGS__); } while(0)
#define glDepthMask(...) do { traceOpenGLCall("glDepthMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDepthMask(__VA_ARGS__); } while(0)
#define glDepthRangef(...) do { traceOpenGLCall("glDepthRangef", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDepthRangef(__VA_ARGS__); } while(0)
#define glDetachShader(...) do { traceOpenGLCall("glDetachShader", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDetachShader(__VA_ARGS__); } while(0)
#define glDisable(...) do { traceOpenGLCall("glDisable", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDisable(__VA_ARGS__); } while(0)
#define glDisableVertexAttribArray(...) do { traceOpenGLCall("glDisableVertexAttribArray", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDisableVertexAttribArray(__VA_ARGS__); } while(0)
#define glDrawArrays(...) do { traceOpenGLCall("glDrawArrays", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDrawArrays(__VA_ARGS__); } while(0)
#define glDrawElements(...) do { traceOpenGLCall("glDrawElements", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glDrawElements(__VA_ARGS__); } while(0)
#define glEnable(...) do { traceOpenGLCall("glEnable", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glEnable(__VA_ARGS__); } while(0)
#define glEnableVertexAttribArray(...) do { traceOpenGLCall("glEnableVertexAttribArray", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glEnableVertexAttribArray(__VA_ARGS__); } while(0)
#define glFinish(...) do { traceOpenGLCall("glFinish", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glFinish(__VA_ARGS__); } while(0)
#define glFlush(...) do { traceOpenGLCall("glFlush", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glFlush(__VA_ARGS__); } while(0)
#define glFramebufferRenderbuffer(...) do { traceOpenGLCall("glFramebufferRenderbuffer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glFramebufferRenderbuffer(__VA_ARGS__); } while(0)
#define glFramebufferTexture2D(...) do { traceOpenGLCall("glFramebufferTexture2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glFramebufferTexture2D(__VA_ARGS__); } while(0)
#define glFrontFace(...) do { traceOpenGLCall("glFrontFace", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glFrontFace(__VA_ARGS__); } while(0)
#define glGenBuffers(...) do { traceOpenGLCall("glGenBuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGenBuffers(__VA_ARGS__); } while(0)
#define glGenerateMipmap(...) do { traceOpenGLCall("glGenerateMipmap", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGenerateMipmap(__VA_ARGS__); } while(0)
#define glGenFramebuffers(...) do { traceOpenGLCall("glGenFramebuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGenFramebuffers(__VA_ARGS__); } while(0)
#define glGenRenderbuffers(...) do { traceOpenGLCall("glGenRenderbuffers", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGenRenderbuffers(__VA_ARGS__); } while(0)
#define glGenTextures(...) do { traceOpenGLCall("glGenTextures", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGenTextures(__VA_ARGS__); } while(0)
#define glGetActiveAttrib(...) do { traceOpenGLCall("glGetActiveAttrib", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetActiveAttrib(__VA_ARGS__); } while(0)
#define glGetActiveUniform(...) do { traceOpenGLCall("glGetActiveUniform", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetActiveUniform(__VA_ARGS__); } while(0)
#define glGetAttachedShaders(...) do { traceOpenGLCall("glGetAttachedShaders", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetAttachedShaders(__VA_ARGS__); } while(0)
#define glGetBooleanv(...) do { traceOpenGLCall("glGetBooleanv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetBooleanv(__VA_ARGS__); } while(0)
#define glGetBufferParameteriv(...) do { traceOpenGLCall("glGetBufferParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetBufferParameteriv(__VA_ARGS__); } while(0)
#define glGetFloatv(...) do { traceOpenGLCall("glGetFloatv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetFloatv(__VA_ARGS__); } while(0)
#define glGetFramebufferAttachmentParameteriv(...) do { traceOpenGLCall("glGetFramebufferAttachmentParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetFramebufferAttachmentParameteriv(__VA_ARGS__); } while(0)
#define glGetIntegerv(...) do { traceOpenGLCall("glGetIntegerv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetIntegerv(__VA_ARGS__); } while(0)
#define glGetProgramiv(...) do { traceOpenGLCall("glGetProgramiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetProgramiv(__VA_ARGS__); } while(0)
#define glGetProgramInfoLog(...) do { traceOpenGLCall("glGetProgramInfoLog", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetProgramInfoLog(__VA_ARGS__); } while(0)
#define glGetRenderbufferParameteriv(...) do { traceOpenGLCall("glGetRenderbufferParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetRenderbufferParameteriv(__VA_ARGS__); } while(0)
#define glGetShaderiv(...) do { traceOpenGLCall("glGetShaderiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetShaderiv(__VA_ARGS__); } while(0)
#define glGetShaderInfoLog(...) do { traceOpenGLCall("glGetShaderInfoLog", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetShaderInfoLog(__VA_ARGS__); } while(0)
#define glGetShaderPrecisionFormat(...) do { traceOpenGLCall("glGetShaderPrecisionFormat", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetShaderPrecisionFormat(__VA_ARGS__); } while(0)
#define glGetShaderSource(...) do { traceOpenGLCall("glGetShaderSource", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetShaderSource(__VA_ARGS__); } while(0)
#define glGetTexParameterfv(...) do { traceOpenGLCall("glGetTexParameterfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetTexParameterfv(__VA_ARGS__); } while(0)
#define glGetTexParameteriv(...) do { traceOpenGLCall("glGetTexParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetTexParameteriv(__VA_ARGS__); } while(0)
#define glGetUniformfv(...) do { traceOpenGLCall("glGetUniformfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetUniformfv(__VA_ARGS__); } while(0)
#define glGetUniformiv(...) do { traceOpenGLCall("glGetUniformiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetUniformiv(__VA_ARGS__); } while(0)
#define glGetVertexAttribfv(...) do { traceOpenGLCall("glGetVertexAttribfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetVertexAttribfv(__VA_ARGS__); } while(0)
#define glGetVertexAttribiv(...) do { traceOpenGLCall("glGetVertexAttribiv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetVertexAttribiv(__VA_ARGS__); } while(0)
#define glGetVertexAttribPointerv(...) do { traceOpenGLCall("glGetVertexAttribPointerv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glGetVertexAttribPointerv(__VA_ARGS__); } while(0)
#define glHint(...) do { traceOpenGLCall("glHint", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glHint(__VA_ARGS__); } while(0)
#define glLineWidth(...) do { traceOpenGLCall("glLineWidth", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glLineWidth(__VA_ARGS__); } while(0)
#define glLinkProgram(...) do { traceOpenGLCall("glLinkProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glLinkProgram(__VA_ARGS__); } while(0)
#define glPixelStorei(...) do { traceOpenGLCall("glPixelStorei", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glPixelStorei(__VA_ARGS__); } while(0)
#define glPolygonOffset(...) do { traceOpenGLCall("glPolygonOffset", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glPolygonOffset(__VA_ARGS__); } while(0)
#define glReadPixels(...) do { traceOpenGLCall("glReadPixels", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glReadPixels(__VA_ARGS__); } while(0)
#define glReleaseShaderCompiler(...) do { traceOpenGLCall("glReleaseShaderCompiler", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glReleaseShaderCompiler(__VA_ARGS__); } while(0)
#define glRenderbufferStorage(...) do { traceOpenGLCall("glRenderbufferStorage", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glRenderbufferStorage(__VA_ARGS__); } while(0)
#define glSampleCoverage(...) do { traceOpenGLCall("glSampleCoverage", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glSampleCoverage(__VA_ARGS__); } while(0)
#define glScissor(...) do { traceOpenGLCall("glScissor", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glScissor(__VA_ARGS__); } while(0)
#define glShaderBinary(...) do { traceOpenGLCall("glShaderBinary", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glShaderBinary(__VA_ARGS__); } while(0)
#define glShaderSource(...) do { traceOpenGLCall("glShaderSource", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glShaderSource(__VA_ARGS__); } while(0)
#define glStencilFunc(...) do { traceOpenGLCall("glStencilFunc", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilFunc(__VA_ARGS__); } while(0)
#define glStencilFuncSeparate(...) do { traceOpenGLCall("glStencilFuncSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilFuncSeparate(__VA_ARGS__); } while(0)
#define glStencilMask(...) do { traceOpenGLCall("glStencilMask", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilMask(__VA_ARGS__); } while(0)
#define glStencilMaskSeparate(...) do { traceOpenGLCall("glStencilMaskSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilMaskSeparate(__VA_ARGS__); } while(0)
#define glStencilOp(...) do { traceOpenGLCall("glStencilOp", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilOp(__VA_ARGS__); } while(0)
#define glStencilOpSeparate(...) do { traceOpenGLCall("glStencilOpSeparate", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glStencilOpSeparate(__VA_ARGS__); } while(0)
#define glTexImage2D(...) do { traceOpenGLCall("glTexImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexImage2D(__VA_ARGS__); } while(0)
#define glTexParameterf(...) do { traceOpenGLCall("glTexParameterf", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexParameterf(__VA_ARGS__); } while(0)
#define glTexParameterfv(...) do { traceOpenGLCall("glTexParameterfv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexParameterfv(__VA_ARGS__); } while(0)
#define glTexParameteri(...) do { traceOpenGLCall("glTexParameteri", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexParameteri(__VA_ARGS__); } while(0)
#define glTexParameteriv(...) do { traceOpenGLCall("glTexParameteriv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexParameteriv(__VA_ARGS__); } while(0)
#define glTexSubImage2D(...) do { traceOpenGLCall("glTexSubImage2D", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glTexSubImage2D(__VA_ARGS__); } while(0)
#define glUniform1f(...) do { traceOpenGLCall("glUniform1f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform1f(__VA_ARGS__); } while(0)
#define glUniform1fv(...) do { traceOpenGLCall("glUniform1fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform1fv(__VA_ARGS__); } while(0)
#define glUniform1i(...) do { traceOpenGLCall("glUniform1i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform1i(__VA_ARGS__); } while(0)
#define glUniform1iv(...) do { traceOpenGLCall("glUniform1iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform1iv(__VA_ARGS__); } while(0)
#define glUniform2f(...) do { traceOpenGLCall("glUniform2f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform2f(__VA_ARGS__); } while(0)
#define glUniform2fv(...) do { traceOpenGLCall("glUniform2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform2fv(__VA_ARGS__); } while(0)
#define glUniform2i(...) do { traceOpenGLCall("glUniform2i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform2i(__VA_ARGS__); } while(0)
#define glUniform2iv(...) do { traceOpenGLCall("glUniform2iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform2iv(__VA_ARGS__); } while(0)
#define glUniform3f(...) do { traceOpenGLCall("glUniform3f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform3f(__VA_ARGS__); } while(0)
#define glUniform3fv(...) do { traceOpenGLCall("glUniform3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform3fv(__VA_ARGS__); } while(0)
#define glUniform3i(...) do { traceOpenGLCall("glUniform3i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform3i(__VA_ARGS__); } while(0)
#define glUniform3iv(...) do { traceOpenGLCall("glUniform3iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform3iv(__VA_ARGS__); } while(0)
#define glUniform4f(...) do { traceOpenGLCall("glUniform4f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform4f(__VA_ARGS__); } while(0)
#define glUniform4fv(...) do { traceOpenGLCall("glUniform4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform4fv(__VA_ARGS__); } while(0)
#define glUniform4i(...) do { traceOpenGLCall("glUniform4i", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform4i(__VA_ARGS__); } while(0)
#define glUniform4iv(...) do { traceOpenGLCall("glUniform4iv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniform4iv(__VA_ARGS__); } while(0)
#define glUniformMatrix2fv(...) do { traceOpenGLCall("glUniformMatrix2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniformMatrix2fv(__VA_ARGS__); } while(0)
#define glUniformMatrix3fv(...) do { traceOpenGLCall("glUniformMatrix3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniformMatrix3fv(__VA_ARGS__); } while(0)
#define glUniformMatrix4fv(...) do { traceOpenGLCall("glUniformMatrix4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUniformMatrix4fv(__VA_ARGS__); } while(0)
#define glUseProgram(...) do { traceOpenGLCall("glUseProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glUseProgram(__VA_ARGS__); } while(0)
#define glValidateProgram(...) do { traceOpenGLCall("glValidateProgram", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glValidateProgram(__VA_ARGS__); } while(0)
#define glVertexAttrib1f(...) do { traceOpenGLCall("glVertexAttrib1f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib1f(__VA_ARGS__); } while(0)
#define glVertexAttrib1fv(...) do { traceOpenGLCall("glVertexAttrib1fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib1fv(__VA_ARGS__); } while(0)
#define glVertexAttrib2f(...) do { traceOpenGLCall("glVertexAttrib2f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib2f(__VA_ARGS__); } while(0)
#define glVertexAttrib2fv(...) do { traceOpenGLCall("glVertexAttrib2fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib2fv(__VA_ARGS__); } while(0)
#define glVertexAttrib3f(...) do { traceOpenGLCall("glVertexAttrib3f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib3f(__VA_ARGS__); } while(0)
#define glVertexAttrib3fv(...) do { traceOpenGLCall("glVertexAttrib3fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib3fv(__VA_ARGS__); } while(0)
#define glVertexAttrib4f(...) do { traceOpenGLCall("glVertexAttrib4f", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib4f(__VA_ARGS__); } while(0)
#define glVertexAttrib4fv(...) do { traceOpenGLCall("glVertexAttrib4fv", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttrib4fv(__VA_ARGS__); } while(0)
#define glVertexAttribPointer(...) do { traceOpenGLCall("glVertexAttribPointer", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glVertexAttribPointer(__VA_ARGS__); } while(0)
#define glViewport(...) do { traceOpenGLCall("glViewport", __FILE__, __PRETTY_FUNCTION__, __LINE__, traceOpenGLCallParams(__VA_ARGS__)); glViewport(__VA_ARGS__); } while(0)

#endif//SP2_ENABLE_OPENGL_TRACING

#endif//SP2_GRAPHICS_OPENGL_H
