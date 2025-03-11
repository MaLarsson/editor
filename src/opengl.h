#ifndef OPENGL_H_
#define OPENGL_H_

#include "common.h"

#define OPENGL_API __stdcall

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef float GLfloat;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef ptrdiff_t GLsizeiptr;
typedef char GLchar;

#define GL_ARRAY_BUFFER 0x8892
#define GL_BLEND 0x0BE2
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_FALSE 0
#define GL_FLOAT 0x1406
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_FUNC_ADD 0x8006
#define GL_LINEAR 0x2601
#define GL_ONE 1
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_RED 0x1903
#define GL_SRC_ALPHA 0x0302
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE0 0x84C0
#define GL_TRIANGLES 0x0004
#define GL_TRUE 1
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_UNSIGNED_BYTE 0x1401
#define GL_VERTEX_SHADER 0x8B31

typedef void *(*GetProcAddressProc)(const char *name);

void init_opengl(GetProcAddressProc load);

typedef GLuint (OPENGL_API *GLCreateShaderProc)(GLenum type);
typedef void (OPENGL_API *GLDeleteShaderProc)(GLuint shader);
typedef void (OPENGL_API *GLShaderSourceProc)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void (OPENGL_API *GLCompileShaderProc)(GLuint shader);
typedef GLuint (OPENGL_API *GLCreateProgramProc)(void);
typedef void (OPENGL_API *GLDeleteProgramProc)(GLuint program);
typedef void (OPENGL_API *GLAttachShaderProc)(GLuint program, GLuint shader);
typedef void (OPENGL_API *GLLinkProgramProc)(GLuint program);
typedef void (OPENGL_API *GLGenVertexArraysProc)(GLsizei n, GLuint *arrays);
typedef void (OPENGL_API *GLGenBuffersProc)(GLsizei n, GLuint *buffers);
typedef void (OPENGL_API *GLDeleteVertexArraysProc)(GLsizei n, const GLuint *arrays);
typedef void (OPENGL_API *GLDeleteBuffersProc)(GLsizei n, const GLuint *buffers);
typedef void (OPENGL_API *GLBindVertexArrayProc)(GLuint array);
typedef void (OPENGL_API *GLBindBufferProc)(GLenum target, GLuint buffer);
typedef void (OPENGL_API *GLBufferDataProc)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (OPENGL_API *GLVertexAttribPointerProc)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (OPENGL_API *GLEnableVertexAttribArrayProc)(GLuint index);
typedef GLint (OPENGL_API *GLGetUniformLocationProc)(GLuint program, const GLchar *name);
typedef void (OPENGL_API *GLUniformMatrix4fvProc)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (OPENGL_API *GLActiveTextureProc)(GLenum texture);
typedef void (OPENGL_API *GLUseProgramProc)(GLuint program);
typedef void (OPENGL_API *GLUniform1iProc)(GLint location, GLint v0);
typedef void (OPENGL_API *GLBindSamplerProc)(GLuint unit, GLuint sampler);
typedef void (OPENGL_API *GLBlendEquationProc)(GLenum mode);
typedef void (OPENGL_API *GLBlendFuncSeparateProc)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (OPENGL_API *GLGenTexturesProc)(GLsizei n, GLuint *textures);
typedef void (OPENGL_API *GLBindTextureProc)(GLenum target, GLuint texture);
typedef void (OPENGL_API *GLPixelStoreiProc)(GLenum pname, GLint param);
typedef void (OPENGL_API *GLTexParameteriProc)(GLenum target, GLenum pname, GLint param);
typedef void (OPENGL_API *GLTexImage2DProc)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * data);
typedef void (OPENGL_API *GLTexSubImage2DProc)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
typedef void (OPENGL_API *GLEnableProc)(GLenum cap);
typedef void (OPENGL_API *GLBlendFuncProc)(GLenum sfactor, GLenum dfactor);
typedef void (OPENGL_API *GLViewportProc)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void (OPENGL_API *GLClearColorProc)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void (OPENGL_API *GLClearProc)(GLbitfield mask);
typedef void (OPENGL_API *GLDrawArraysProc)(GLenum mode, GLint first, GLsizei count);

extern GLCreateShaderProc glCreateShader;
extern GLDeleteShaderProc glDeleteShader;
extern GLShaderSourceProc glShaderSource;
extern GLCompileShaderProc glCompileShader;
extern GLCreateProgramProc glCreateProgram;
extern GLDeleteProgramProc glDeleteProgram;
extern GLAttachShaderProc glAttachShader;
extern GLLinkProgramProc glLinkProgram;
extern GLGenVertexArraysProc glGenVertexArrays;
extern GLGenBuffersProc glGenBuffers;
extern GLDeleteVertexArraysProc glDeleteVertexArrays;
extern GLDeleteBuffersProc glDeleteBuffers;
extern GLBindVertexArrayProc glBindVertexArray;
extern GLBindBufferProc glBindBuffer;
extern GLBufferDataProc glBufferData;
extern GLVertexAttribPointerProc glVertexAttribPointer;
extern GLEnableVertexAttribArrayProc glEnableVertexAttribArray;
extern GLGetUniformLocationProc glGetUniformLocation;
extern GLUniformMatrix4fvProc glUniformMatrix4fv;
extern GLActiveTextureProc glActiveTexture;
extern GLUseProgramProc glUseProgram;
extern GLUniform1iProc glUniform1i;
extern GLBindSamplerProc glBindSampler;
extern GLBlendEquationProc glBlendEquation;
extern GLBlendFuncSeparateProc glBlendFuncSeparate;
extern GLGenTexturesProc glGenTextures;
extern GLBindTextureProc glBindTexture;
extern GLPixelStoreiProc glPixelStorei;
extern GLTexParameteriProc glTexParameteri;
extern GLTexImage2DProc glTexImage2D;
extern GLTexSubImage2DProc glTexSubImage2D;
extern GLEnableProc glEnable;
extern GLBlendFuncProc glBlendFunc;
extern GLViewportProc glViewport;
extern GLClearColorProc glClearColor;
extern GLClearProc glClear;
extern GLDrawArraysProc glDrawArrays;

#endif // OPENGL_H_
