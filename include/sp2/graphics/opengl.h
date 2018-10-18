#ifndef SP2_GRAPHICS_OPENGL_H
#define SP2_GRAPHICS_OPENGL_H

//TODO: Replace glew and only expose OpenGL ES 2.0 compattible functions, for maximum compatibility with all targets.
#include <GL/glew.h>

/** Below is the available OpenGL ES 2.0 functions from the ES2.0 cheat sheet.

Errors [2.5]
enum glGetError( void ); //Returns one of the following:
//Enum argument out of range
#define GL_INVALID_ENUM 0x0500
//Framebuffer is incomplete
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
//Numeric argument out of range
#define GL_INVALID_VALUE 0x0501
//Operation illegal in current state
#define GL_INVALID_OPERATION 0x0502
//Not enough memory left to execute command
#define GL_OUT_OF_MEMORY 0x0505
//No error encountered
#define GL_NO_ERROR 0


Buffer Objects [2.9]
Buffer objects hold vertex array data or indices in high-performance server memory.
void glGenBuffers(sizei n, uint *buffers);
void glDeleteBuffers(sizei n, const uint *buffers);
Creating and Binding Buffer Objects
void glBindBuffer(enum target, uint buffer);
target: ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER
Creating Buffer Object Data Stores
void glBufferData(enum target, sizeiptr size, const void *data, enum usage);
usage: STATIC_DRAW, STREAM_DRAW, DYNAMIC_DRAW
Updating Buffer Object Data Stores
void glBufferSubData(enum target, intptr offset, sizeiptr size, const void *data);
target: ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER
Buffer Object Queries [6.1.6, 6.1.3]
boolean glIsBuffer(uint buffer);
void glGetBufferParameteriv(enum target, enum value, T data);
target: ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER
value: BUFFER_SIZE, BUFFER_USAGE


Viewport and Clipping
Controlling the Viewport [2.12.1]
void glDepthRangef(clampf n, clampf f);
void glViewport(int x, int y, sizei w, sizei h);


Reading Pixels [4.3.1]
void glReadPixels(int x, int y, sizei width, sizei height, enum format, enum type, void *data);
format: RGBA type: UNSIGNED_BYTE
Note: ReadPixels() also accepts a queriable implementation defined format/type combination, see [4.3.1].


Texturing [3.7]
Shaders support texturing using at least MAX_VERTEX_TEXTURE_IMAGE_UNITS images for vertex shaders and at least MAX_TEXTURE_IMAGE_UNITS images for fragment shaders.
void glActiveTexture(enum texture); texture: [TEXTURE0..TEXTUREi] where i = MAX_COMBINED_TEXTURE_IMAGE_UNITS-1
Texture Image Specification [3.7.1]
void glTexImage2D(enum target, int level, int internalformat, sizei width, sizei height, int border, enum format, enum type, void *data);
target: TEXTURE_2D, TEXTURE_CUBE_MAP_POSITIVE_{X,Y,Z} TEXTURE_CUBE_MAP_NEGATIVE_{X,Y,Z}
internalformat: ALPHA, LUMINANCE, LUMINANCE_ALPHA, RGB, RGBA
format: ALPHA, RGB, RGBA, LUMINANCE, LUMINANCE_ALPHA
type: UNSIGNED_BYTE, UNSIGNED_SHORT_5_6_5,
UNSIGNED_SHORT_4_4_4_4, UNSIGNED_SHORT_5_5_5_1
Conversion from RGBA pixel components to internal
texture components:
Base Internal Format RGBA Internal Components
ALPHA A A
LUMINANCE R L
LUMINANCE _ALPHA R, A L, A
RGB R, G, B R, G, B
RGBA R, G, B, A R, G, B, A
Alt. Texture Image Specification Commands [3.7.2]
Texture images may also be specified using image data taken
directly from the framebuffer, and rectangular subregions of
existing texture images may be respecified.
void glCopyTexImage2D(enum target, int level, enum internalformat, int x, int y, sizei width, sizei height, int border);
target: TEXTURE_2D, TEXTURE_CUBE_MAP_POSITIVE_{X, Y, Z},
TEXTURE_CUBE_MAP_NEGATIVE_{X, Y, Z}
internalformat: See TexImage2D
void glTexSubImage2D(enum target, int level, int xoffset, int yoffset, sizei width, sizei height, enum format, enum type, void *data);
target: TEXTURE_CUBE_MAP_POSITIVE_{X, Y, Z},
TEXTURE_CUBE_MAP_NEGATIVE_{X, Y, Z}
format and type: See TexImage2D
void glCopyTexSubImage2D(enum target, int level, int xoffset, int yoffset, int x, int y, sizei width, sizei height);
target: TEXTURE_2D, TEXTURE_CUBE_MAP_POSITIVE_{X, Y, Z},
TEXTURE_CUBE_MAP_NEGATIVE_{X, Y, Z}
format and type: See TexImage2D
Compressed Texture Images [3.7.3]
void glCompressedTexImage2D(enum target, int level, enum internalformat, sizei width, sizei height, int border, sizei imageSize, void *data);
target and internalformat: See TexImage2D
void glCompressedTexSubImage2D(enum target, int level, int xoffset, int yoffset, sizei width, sizei height, enum format, sizei imageSize, void *data);
target and internalformat: See TexImage2D
Texture Parameters [3.7.4]
void glTexParameter{if}(enum target, enum pname, T param);
void glTexParameter{if}v(enum target, enum pname, T params);
target: TEXTURE_2D, TEXTURE_CUBE_MAP
pname: TEXTURE_WRAP_{S, T}, TEXTURE_{MIN, MAG}_FILTER
Manual Mipmap Generation [3.7.11]
void glGenerateMipmap(enum target);
target: TEXTURE_2D, TEXTURE_CUBE_MAP
Texture Objects [3.7.13]
void glBindTexture(enum target, uint texture);
void glDeleteTextures(sizei n, uint *textures);
void glGenTextures(sizei n, uint *textures);
Enumerated Queries [6.1.3]
void glGetTexParameter{if}v(enum target, enum value, T data);
target: TEXTURE_2D, TEXTURE_CUBE_MAP
value: TEXTURE_WRAP_{S, T}, TEXTURE_{MIN, MAG}_FILTER
Texture Queries [6.1.4]
boolean glIsTexture(uint texture);


Vertices
Current Vertex State [2.7]
void glVertexAttrib{1234}{f}(uint index, T values);
void glVertexAttrib{1234}{f}v(uint index, T values);
Vertex Arrays [2.8]
Vertex data may be sourced from arrays that are stored in application
memory (via a pointer) or faster GPU memory (in a buffer object).
void glVertexAttribPointer(uint index, int size, enum type, boolean normalized, sizei stride, const void *pointer);
type: BYTE, UNSIGNED_BYTE, SHORT, UNSIGNED_SHORT, FIXED, FLOAT
index: [0, MAX_VERTEX_ATTRIBS - 1]
If an ARRAY_BUFFER is bound, the attribute will be read from the
bound buffer, and pointer is treated as an offset within the buffer.
void glEnableVertexAttribArray(uint index);
void glDisableVertexAttribArray(uint index);
index: [0, MAX_VERTEX_ATTRIBS - 1]
void glDrawArrays(enum mode, int first, sizei count);
void glDrawElements(enum mode, sizei count, enum type, void *indices);
mode: POINTS, LINE_STRIP, LINE_LOOP, LINES, TRIANGLE_STRIP,
TRIANGLE_FAN, TRIANGLES
type: UNSIGNED_BYTE, UNSIGNED_SHORT
If an ELEMENT_ARRAY_BUFFER is bound, the indices will be read
from the bound buffer, and indices is treated as an offset within
the buffer.


Rasterization [3]
Points [3.3]
Point size is taken from the shader builtin gl_PointSize and
clamped to the implementation-dependent point size range.
Line Segments [3.4]
void glLineWidth(float width);
Polygons [3.5]
void glFrontFace(enum dir);
dir: CCW, CW
void glCullFace(enum mode);
mode: FRONT, BACK, FRONT_AND_BACK
Enable/Disable(CULL_FACE)
void glPolygonOffset(float factor, float units);
Enable/Disable(POLYGON_OFFSET_FILL)


Pixel Rectangles [3.6, 4.3]
void PixelStorei(enum pname, int param);
pname: UNPACK_ALIGNMENT, PACK_ALIGNMENT


Shaders and Programs
Shader Objects [2.10.1]
uint CreateShader(enum type);
type: VERTEX_SHADER, FRAGMENT_SHADER
void glShaderSource(uint shader, sizei count, const char **string, const int *length);
void glCompileShader(uint shader);
void glReleaseShaderCompiler(void);
void glDeleteShader(uint shader);
Loading Shader Binaries [2.10.2]
void glShaderBinary(sizei count, const uint *shaders, enum binaryformat, const void *binary, sizei length);
Program Objects [2.10.3]
uint glCreateProgram(void);
void glAttachShader(uint program, uint shader);
void glDetachShader(uint program, uint shader);
void glLinkProgram(uint program);
void glUseProgram(uint program);
void glDeleteProgram(uint program);
Shader Variables [2.10.4]
Vertex Attributes
void glGetActiveAttrib(uint program, uint index, sizei bufSize, sizei *length, int *size, enum *type, char *name);
*type returns: FLOAT, FLOAT_VEC{2,3,4}, FLOAT_MAT{2,3,4}
int glGetAttribLocation(uint program, const char *name);
void glBindAttribLocation(uint program, uint index, const char *name);
Uniform Variables
int glGetUniformLocation(uint program, const char *name);
void glGetActiveUniform(uint program, uint index, sizei bufSize, sizei *length, int *size, enum *type, char *name);
*type: FLOAT, FLOAT_VEC{2,3,4}, INT, INT_VEC{2,3,4}, BOOL,
BOOL_VEC{2,3,4}, FLOAT_MAT{2,3,4}, SAMPLER_2D,
SAMPLER_CUBE
void glUniform{1234}{if}(int location, T value);
void glUniform{1234}{if}v(int location, sizei count, T value);
void glUniformMatrix{234}fv(int location, sizei count, boolean transpose, const float *value);
transpose: FALSE
Shader Execution (Validation) [2.10.5]
void glValidateProgram(uint program);
Shader Queries
Shader Queries [6.1.8]
boolean glIsShader(uint shader);
void glGetShaderiv(uint shader, enum pname, int *params);
pname: SHADER_TYPE, DELETE_STATUS, COMPILE_STATUS,
INFO_LOG_LENGTH, SHADER_SOURCE_LENGTH
void glGetAttachedShaders(uint program, sizei maxCount, sizei *count, uint *shaders);
void glGetShaderInfoLog(uint shader, sizei bufSize, sizei *length, char *infoLog);
void glGetShaderSource(uint shader, sizei bufSize,
sizei *length, char *source);
void glGetShaderPrecisionFormat(enum shadertype, enum precisiontype, int *range, int *precision);
shadertype: VERTEX_SHADER, FRAGMENT_SHADER
precision: LOW_FLOAT, MEDIUM_FLOAT, HIGH_FLOAT, LOW_INT,
MEDIUM_INT, HIGH_INT
void glGetVertexAttribfv(uint index, enum pname, float *params);
pname: CURRENT_VERTEX_ATTRIB , VERTEX_ATTRIB_ARRAY_x
(where x may be BUFFER_BINDING, ENABLED, SIZE, STRIDE, TYPE, NORMALIZED)
void glGetVertexAttribiv(uint index, enum pname, int *params);
pname: CURRENT_VERTEX_ATTRIB , VERTEX_ATTRIB_ARRAY_x
(where x may be BUFFER_BINDING, ENABLED, SIZE, STRIDE, TYPE, NORMALIZED)
void glGetVertexAttribPointerv(uint index, enum pname, void **pointer);
pname: VERTEX_ATTRIB_ARRAY_POINTER
void glGetUniformfv(uint program, int location, float *params)
void glGetUniformiv(uint program, int location, int *params)
Program Queries [6.1.8]
boolean glIsProgram(uint program);
void glGetProgramiv(uint program, enum pname, int *params);
pname: DELETE_STATUS, LINK_STATUS, VALIDATE_STATUS, INFO_LOG_LENGTH, ATTACHED_SHADERS, ACTIVE_ATTRIBUTES, ACTIVE_ATTRIBUTE_MAX_LENGTH, ACTIVE_UNIFORMS, ACTIVE_UNIFORM_MAX_LENGTH
void glGetProgramInfoLog(uint program, sizei bufSize, sizei *length, char *infoLog);


Per-Fragment Operations
Scissor Test [4.1.2]
glEnable/glDisable(SCISSOR_TEST)
void glScissor(int left, int bottom, sizei width, sizei height );
Multisample Fragment Operations [4.1.3]
glEnable/glDisable(cap)
cap: SAMPLE_ALPHA_TO_COVERAGE, SAMPLE_COVERAGE
void glSampleCoverage(clampf value, boolean invert);
Stencil Test [4.1.4]
glEnable/glDisable(STENCIL_TEST)
void glStencilFunc(enum func, int ref, uint mask);
void glStencilFuncSeparate(enum face, enum func, int ref, uint mask);
void glStencilOp(enum sfail, enum dpfail, enum dppass);
void glStencilOpSeparate(enum face, enum sfail, enum dpfail, enum dppass);
face: FRONT, BACK, FRONT_AND_BACK
sfail, dpfail, and dppass: KEEP, ZERO, REPLACE, INCR, DECR, INVERT, INCR_WRAP, DECR_WRAP
func: NEVER, ALWAYS, LESS, LEQUAL, EQUAL, GREATER, GEQUAL, NOTEQUAL
Depth Buffer Test [4.1.5]
glEnable/glDisable(DEPTH_TEST)
void glDepthFunc(enum func);
func: NEVER, ALWAYS, LESS, LEQUAL, EQUAL, GREATER, GEQUAL, NOTEQUAL
Blending [4.1.6]
glEnable/glDisable(BLEND) (applies to all draw buffers)
void glBlendEquation(enum mode);
void glBlendEquationSeparate(enum modeRGB, enum modeAlpha);
mode, modeRGB, and modeAlpha: FUNC_ADD, FUNC_SUBTRACT, FUNC_REVERSE_SUBTRACT
void glBlendFuncSeparate(enum srcRGB, enum dstRGB, enum srcAlpha, enum dstAlpha);
void glBlendFunc(enum src, enum dst);
dst, dstRGB, and dstAlpha: ZERO, ONE, [ONE_MINUS_]SRC_COLOR,
[ONE_MINUS_]DST_COLOR, [ONE_MINUS_]SRC_ALPHA,
[ONE_MINUS_]DST_ALPHA, [ONE_MINUS_]CONSTANT_COLOR,
[ONE_MINUS_]CONSTANT_ALPHA
src, srcRGB, srcAlpha: same for dst, plus SRC_ALPHA_SATURATE
void glBlendColor(clampf red, clampf green, clampf blue, clampf alpha);
Dithering [4.1.7]
glEnable/glDisable(DITHER) 


Whole Framebuffer Operations
Fine Control of Buffer Updates [4.2.2]
void glColorMask(boolean r, boolean g, boolean b, boolean a);
void glDepthMask(boolean mask);
void glStencilMask(uint mask);
void glStencilMaskSeparate(enum face, uint mask);
face: FRONT, BACK, FRONT_AND_BACK
Clearing the Buffers [4.2.3]
void glClear(bitfield buf);
buf: Bitwise OR of COLOR_BUFFER_BIT, DEPTH_BUFFER_BIT, STENCIL_BUFFER_BIT
void glClearColor(clampf r, clampf g, clampf b, clampf a);
void glClearDepthf(clampf d);
void glClearStencil(int s);


Special Functions
Flush and Finish [5.1]
Flush guarantees that commands issued so
far will eventually complete. Finish blocks
until all commands issued so far have
completed.
void glFlush(void);
void glFinish(void);
Hints [5.2]
Hint controls certain aspects of GL behavior.
void glHint(enum target, enum hint);
target: GENERATE_MIPMAP_HINT
hint: FASTEST, NICEST, DONT_CARE


State and State Requests
A complete list of symbolic constants for
states is shown in the tables in [6.2].
Simple Queries [6.1.1]
void glGetBooleanv(enum value, boolean *data);
void glGetIntegerv(enum value, int *data);
void glGetFloatv(enum value, float *data);
boolean glIsEnabled(enum value);
Pointer and String Queries [6.1.5]
ubyte *glGetString(enum name); name: VENDOR, RENDERER, VERSION, SHADING_LANGUAGE_VERSION, EXTENSIONS


Framebuffer Objects
Binding & Managing Framebuffer Objects [4.4.1]
void glBindFramebuffer(enum target, uint framebuffer);
target: FRAMEBUFFER
void glDeleteFramebuffers(sizei n, uint *framebuffers);
void glGenFramebuffers(sizei n, uint *framebuffers);
Renderbuffer Objects [4.4.2]
void glBindRenderbuffer(enum target, uint renderbuffer);
target: RENDERBUFFER
void glDeleteRenderbuffers(sizei n, const uint *renderbuffers);
void glGenRenderbuffers(sizei n, uint *renderbuffers);
void glRenderbufferStorage(enum target, enum internalformat, sizei width, sizei height);
target: RENDERBUFFER
 internalformat: DEPTH_COMPONENT16, RGBA4, RGB5_A1,
RGB565, STENCIL_INDEX8
Attaching Renderbuffer Images to Framebuffer
void glFramebufferRenderbuffer(enum target, enum attachment, enum renderbuffertarget, uint renderbuffer);
target: FRAMEBUFFER
attachment: COLOR_ATTACHMENT0, DEPTH_ATTACHMENT,
STENCIL_ATTACHMENT
renderbuffertarget: RENDERBUFFER
Attaching Texture Images to a Framebuffer
void glFramebufferTexture2D(enum target, enum attachment, enum textarget, uint texture, int level);
textarget: TEXTURE_2D, TEXTURE_CUBE_MAP_POSITIVE{X, Y, Z},
TEXTURE_CUBE_MAP_NEGATIVE{X, Y, Z},
target: FRAMEBUFFER
attachment: COLOR_ATTACHMENT0, DEPTH_ATTACHMENT,
STENCIL_ATTACHMENT
Framebuffer Completeness [4.4.5]
enum glCheckFramebufferStatus(enum target);
target: FRAMEBUFFER
returns: FRAMEBUFFER_COMPLETE or a constant indicating which value violates framebuffer completeness
Framebuffer Object Queries [6.1.3, 6.1.7]
boolean IsFramebuffer(uint framebuffer);
void glGetFramebufferAttachmentParameteriv(enum target, enum attachment, enum pname, int *params);
target: FRAMEBUFFER
attachment: COLOR_ATTACHMENT0, DEPTH_ATTACHMENT,
STENCIL_ATTACHMENT
pname: FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE
Renderbuffer Object Queries [6.1.3, 6.1.7]
boolean glIsRenderbuffer(uint renderbuffer);
void glGetRenderbufferParameteriv(enum target, enum pname, int *params);
target: RENDERBUFFER
pname: RENDERBUFFER_x (where x may be WIDTH, HEIGHT, RED_SIZE, GREEN_SIZE, BLUE_SIZE, ALPHA_SIZE, DEPTH_SIZE, STENCIL_SIZE, INTERNAL_FORMAT)
*/

#endif//SP2_GRAPHICS_OPENGL_H
