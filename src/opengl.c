#include "common.h"
#include "opengl.h"

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
