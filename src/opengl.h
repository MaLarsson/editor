#ifndef OPENGL_H_
#define OPENGL_H_

#define OPENGL_API __stdcall

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

#define GL_VERTEX_SHADER   0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_TEXTURE0        0x84C0
#define GL_CLAMP_TO_EDGE   0x812F
#define GL_ARRAY_BUFFER    0x8892
#define GL_DYNAMIC_DRAW    0x88E8
#define GL_FUNC_ADD        0x8006

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

typedef void *(*GetProcAddressProc)(const char *name);

extern void init_opengl(GetProcAddressProc load);

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

#endif // OPENGL_H_

#ifdef OPENGL_IMPLEMENTATION

GLCreateShaderProc glCreateShader = NULL;
GLDeleteShaderProc glDeleteShader = NULL;
GLShaderSourceProc glShaderSource = NULL;
GLCompileShaderProc glCompileShader = NULL;
GLCreateProgramProc glCreateProgram = NULL;
GLDeleteProgramProc glDeleteProgram = NULL;
GLAttachShaderProc glAttachShader = NULL;
GLLinkProgramProc glLinkProgram = NULL;
GLGenVertexArraysProc glGenVertexArrays = NULL;
GLGenBuffersProc glGenBuffers = NULL;
GLDeleteVertexArraysProc glDeleteVertexArrays = NULL;
GLDeleteBuffersProc glDeleteBuffers = NULL;
GLBindVertexArrayProc glBindVertexArray = NULL;
GLBindBufferProc glBindBuffer = NULL;
GLBufferDataProc glBufferData = NULL;
GLVertexAttribPointerProc glVertexAttribPointer = NULL;
GLEnableVertexAttribArrayProc glEnableVertexAttribArray = NULL;
GLGetUniformLocationProc glGetUniformLocation = NULL;
GLUniformMatrix4fvProc glUniformMatrix4fv = NULL;
GLActiveTextureProc glActiveTexture = NULL;
GLUseProgramProc glUseProgram = NULL;
GLUniform1iProc glUniform1i = NULL;
GLBindSamplerProc glBindSampler = NULL;
GLBlendEquationProc glBlendEquation = NULL;
GLBlendFuncSeparateProc glBlendFuncSeparate = NULL;

void init_opengl(GetProcAddressProc load) {
    glCreateShader = load("glCreateShader");
    glDeleteShader = load("glDeleteShader");
    glShaderSource = load("glShaderSource");
    glCompileShader = load("glCompileShader");
    glCreateProgram = load("glCreateProgram");
    glDeleteProgram = load("glDeleteProgram");
    glAttachShader = load("glAttachShader");
    glLinkProgram = load("glLinkProgram");
    glGenVertexArrays = load("glGenVertexArrays");
    glGenBuffers = load("glGenBuffers");
    glDeleteVertexArrays = load("glDeleteVertexArrays");
    glDeleteBuffers = load("glDeleteBuffers");
    glBindVertexArray = load("glBindVertexArray");
    glBindBuffer = load("glBindBuffer");
    glBufferData = load("glBufferData");
    glVertexAttribPointer = load("glVertexAttribPointer");
    glEnableVertexAttribArray = load("glEnableVertexAttribArray");
    glGetUniformLocation = load("glGetUniformLocation");
    glUniformMatrix4fv = load("glUniformMatrix4fv");
    glActiveTexture = load("glActiveTexture");
    glUseProgram = load("glUseProgram");
    glUniform1i = load("glUniform1i");
    glBindSampler = load("glBindSampler");
    glBlendEquation = load("glBlendEquation");
    glBlendFuncSeparate = load("glBlendFuncSeparate");
}

#endif // OPENGL_IMPLEMENTATION
