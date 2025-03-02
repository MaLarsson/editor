#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <gl/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB   0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB  0x2011
#define WGL_PIXEL_TYPE_ARB     0x2013
#define WGL_COLOR_BITS_ARB     0x2014
#define WGL_DEPTH_BITS_ARB     0x2022
#define WGL_STENCIL_BITS_ARB   0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB         0x202B

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9

#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126

static bool window_should_close = false;

typedef BOOL (WINAPI *WGLSwapIntervalExtProc)(int interval);
typedef HGLRC (WINAPI *WGLCreateContextAttribsARBProc)(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL (WINAPI *WGLChoosePixelFormatARBProc)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static WGLSwapIntervalExtProc wglSwapIntervalEXT;
static WGLCreateContextAttribsARBProc wglCreateContextAttribsARB;
static WGLChoosePixelFormatARBProc wglChoosePixelFormatARB;

static const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in vec4 color;\n"
    "\n"
    "uniform mat4 projection;\n"
    "\n"
    "out vec4 vertexColor;\n"
    "out vec2 vertexUV;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(pos, 0.0, 1.0);\n"
    "    vertexUV = uv;\n"
    "    vertexColor = color;\n"
    "}\n";

static const char *fragment_shader =
    "#version 330 core\n"
    "in vec4 vertexColor;\n"
    "in vec2 vertexUV;\n"
    "\n"
    "uniform sampler2D Texture;\n"
    "\n"
    "out vec4 color;\n"
    "\n"
    "void main() {\n"
    "    color = vec4(vertexColor.xyz, texture(Texture, vertexUV));\n"
    "}\n";

#define OPENGL_API __stdcall

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

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

static GLCreateShaderProc glCreateShader = NULL;
static GLDeleteShaderProc glDeleteShader = NULL;
static GLShaderSourceProc glShaderSource = NULL;
static GLCompileShaderProc glCompileShader = NULL;
static GLCreateProgramProc glCreateProgram = NULL;
static GLDeleteProgramProc glDeleteProgram = NULL;
static GLAttachShaderProc glAttachShader = NULL;
static GLLinkProgramProc glLinkProgram = NULL;
static GLGenVertexArraysProc glGenVertexArrays = NULL;
static GLGenBuffersProc glGenBuffers = NULL;
static GLDeleteVertexArraysProc glDeleteVertexArrays = NULL;
static GLDeleteBuffersProc glDeleteBuffers = NULL;
static GLBindVertexArrayProc glBindVertexArray = NULL;
static GLBindBufferProc glBindBuffer = NULL;
static GLBufferDataProc glBufferData = NULL;
static GLVertexAttribPointerProc glVertexAttribPointer = NULL;
static GLEnableVertexAttribArrayProc glEnableVertexAttribArray = NULL;
static GLGetUniformLocationProc glGetUniformLocation = NULL;
static GLUniformMatrix4fvProc glUniformMatrix4fv = NULL;
static GLActiveTextureProc glActiveTexture = NULL;
static GLUseProgramProc glUseProgram = NULL;
static GLUniform1iProc glUniform1i = NULL;
static GLBindSamplerProc glBindSampler = NULL;
static GLBlendEquationProc glBlendEquation = NULL;
static GLBlendFuncSeparateProc glBlendFuncSeparate = NULL;

#define GL_VERTEX_SHADER   0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_TEXTURE0        0x84C0
#define GL_CLAMP_TO_EDGE   0x812F
#define GL_ARRAY_BUFFER    0x8892
#define GL_DYNAMIC_DRAW    0x88E8
#define GL_FUNC_ADD        0x8006

typedef void *(*GetProcAddressProc)(const char *name);

static void init_opengl(GetProcAddressProc load) {
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

static GLint projection_location = -1;
static GLint texture_location = -1;

static void opengl_compile_shaders(void) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_shader, NULL);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(program);

    projection_location = glGetUniformLocation(program, "projection");
    texture_location = glGetUniformLocation(program, "Texture");
}

static void win32_init_opengl_extensions(void) {
    WNDCLASSA dummy_window_class = {0};
    dummy_window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    dummy_window_class.lpfnWndProc = DefWindowProcA;
    dummy_window_class.hInstance = GetModuleHandle(NULL);
    dummy_window_class.lpszClassName = "wgl_loader";

    if (!RegisterClassA(&dummy_window_class)) {
        //fatal_error("Failed to register dummy OpenGL window.");
    }

    HWND dummy_window = CreateWindowExA(0, dummy_window_class.lpszClassName, "", 0,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                        0, 0, dummy_window_class.hInstance, 0);
    if (!dummy_window) {
        //fatal_error("Failed to create dummy OpenGL window.");
    }

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR desired_pixel_format = {0};
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.cStencilBits = 8;

    int pixel_format_index = ChoosePixelFormat(dummy_dc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR pixel_format;
    DescribePixelFormat(dummy_dc, pixel_format_index, sizeof(pixel_format), &pixel_format);
    SetPixelFormat(dummy_dc, pixel_format_index, &pixel_format);

    HGLRC dummy_rc = wglCreateContext(dummy_dc);
    if (!wglMakeCurrent(dummy_dc, dummy_rc)) {
        //printf("unable to create opengl context\n");
        //window_should_close = true;
    }

    wglSwapIntervalEXT = (WGLSwapIntervalExtProc)wglGetProcAddress("wglSwapIntervalEXT");
    wglCreateContextAttribsARB = (WGLCreateContextAttribsARBProc)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (WGLChoosePixelFormatARBProc)wglGetProcAddress("wglChoosePixelFormatARB");

    init_opengl((GetProcAddressProc)wglGetProcAddress);

    if (!wglSwapIntervalEXT || !wglCreateContextAttribsARB || !wglChoosePixelFormatARB) {
        //printf("unable to load required opengl extensions\n");
        //window_should_close = true;
    }

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_rc);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    UnregisterClassA(dummy_window_class.lpszClassName, dummy_window_class.hInstance);
}

static HGLRC win32_init_opengl(HDC window_dc) {
    win32_init_opengl_extensions();

    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,           GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,           GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,            GL_TRUE,
        WGL_ACCELERATION_ARB,             WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,               WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,               32,
        WGL_DEPTH_BITS_ARB,               24,
        WGL_STENCIL_BITS_ARB,             8,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
        0,
    };

    int pixel_format_index;
    UINT num_formats;
    wglChoosePixelFormatARB(window_dc, pixel_format_attribs, 0, 1, &pixel_format_index, &num_formats);
    if (!num_formats) {
        // ...
    }

    PIXELFORMATDESCRIPTOR pixel_format;
    DescribePixelFormat(window_dc, pixel_format_index, sizeof(pixel_format), &pixel_format);
    SetPixelFormat(window_dc, pixel_format_index, &pixel_format);

    int opengl_context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    HGLRC opengl_rc = wglCreateContextAttribsARB(window_dc, 0, opengl_context_attribs);

    if (!wglMakeCurrent(window_dc, opengl_rc)) {
        printf("unable to create opengl context\n");
        window_should_close = true;
    }

    return opengl_rc;
}

typedef struct Vec2 {
    float x;
    float y;
} Vec2;

typedef struct Vec3 {
    float x;
    float y;
    float z;
} Vec3;

typedef struct Vertex {
    float x;
    float y;
    float u;
    float v;
    uint32_t color;
} Vertex;

#define GLYPH_COUNT 255

typedef struct Glyph {
    float x;
    float y;
    float x_bearing;
    float y_bearing;
    float advance;
    float width;
    float height;
} Glyph;

typedef struct FontAtlas {
    float texture_width;
    float texture_height;
    Glyph glyphs[GLYPH_COUNT];

    float font_height;
    float max_advance;
    float ascent;
    float descent;
} FontAtlas;

static size_t glyph_index(char c) {
    size_t index = c - ' ';
    // TODO(marla): return the glyph index for box?
    assert(index < GLYPH_COUNT && "invalid glyph index");
    return index;
}

static FontAtlas create_font_texture() {
    FontAtlas atlas = {0};

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texture_location, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        printf("error loading freetype library\n");
    }

    FT_Face face;
    if (FT_New_Face(library, "C:/Windows/Fonts/consola.ttf", 0, &face)) {
        printf("error loading font\n");
    }

    uint32_t pt_size = 18;
    FT_Size_RequestRec size = {0};
    size.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = (pt_size << 6);
    FT_Request_Size(face, &size);
    atlas.font_height = face->size->metrics.height / 64.0f;
    atlas.max_advance = face->size->metrics.max_advance / 64.0f;
    atlas.ascent = face->size->metrics.ascender / 64.0f;
    atlas.descent = face->size->metrics.descender / 64.0f;

    atlas.texture_width = 0;
    atlas.texture_height = 0;

    atlas.texture_width += 3;

    for (char c = ' '; c < '~'; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);

        atlas.texture_width += face->glyph->bitmap.width + 1;
        GLsizei h = face->glyph->bitmap.rows;
        if (h > atlas.texture_height) atlas.texture_height = h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.texture_width, atlas.texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    GLsizei x_offset = 0;

    uint8_t white_buffer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RED, GL_UNSIGNED_BYTE, white_buffer);
    x_offset += 2;

    for (char c = ' '; c < '~'; c++) {
        FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);

        size_t index = glyph_index(c);
        Glyph *glyph = &atlas.glyphs[index];
        glyph->x = x_offset;
        glyph->y = 0;
        glyph->x_bearing = face->glyph->bitmap_left;
        glyph->y_bearing = face->glyph->bitmap_top;
        glyph->advance = (face->glyph->advance.x >> 6);
        glyph->width = face->glyph->bitmap.width;
        glyph->height = face->glyph->bitmap.rows;

        x_offset += glyph->width;
        glTexSubImage2D(GL_TEXTURE_2D, 0, glyph->x, glyph->y, glyph->width, glyph->height, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return atlas;
}

#define GREEN 0xFF00FF00
#define RED   0xFF0000FF
#define BLUE  0xFFFF0000
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000

static void render_cursor(FontAtlas atlas, float *x, float y, Vertex *buffer) {
    float line_gap = atlas.font_height - (atlas.ascent - atlas.descent);
    float h = atlas.font_height - line_gap;
    float w = atlas.max_advance;
    float x_pos = *x;
    float y_pos = y - h;

    float x_uv_from = 0;
    float x_uv_to = 0;
    float y_uv_from = 0;
    float y_uv_to = 0;

    Vertex vertices[6] = {
        {x_pos,     y_pos,     x_uv_from, y_uv_to,   GREEN},
        {x_pos,     y_pos + h, x_uv_from, y_uv_from, GREEN},
        {x_pos + w, y_pos,     x_uv_to,   y_uv_to,   GREEN},

        {x_pos + w, y_pos,     x_uv_to,   y_uv_to,   GREEN},
        {x_pos,     y_pos + h, x_uv_from, y_uv_from, GREEN},
        {x_pos + w, y_pos + h, x_uv_to,   y_uv_from, GREEN},
    };

    *x = *x + atlas.max_advance;
    memcpy(buffer, vertices, sizeof(vertices));
}

static void render_char(FontAtlas atlas, float *x, float y, char c, Vertex *buffer, uint32_t color) {
    int index = glyph_index(c);
    Glyph *glyph = &atlas.glyphs[index];
    float padding = (atlas.font_height - atlas.ascent + atlas.descent) / 2;

    float char_h = glyph->height;
    float char_w = glyph->width;
    float x_pos = *x + glyph->x_bearing;
    float y_pos = y - padding - atlas.ascent - (char_h - glyph->y_bearing);

    float x_uv_from = glyph->x / atlas.texture_width;
    float x_uv_to = (glyph->x + glyph->width) / atlas.texture_width;
    float y_uv_from = glyph->y / atlas.texture_height;
    float y_uv_to = (glyph->y + glyph->height) / atlas.texture_height;

    Vertex vertices[] = {
        {x_pos,          y_pos,          x_uv_from, y_uv_to,   color},
        {x_pos,          y_pos + char_h, x_uv_from, y_uv_from, color},
        {x_pos + char_w, y_pos,          x_uv_to,   y_uv_to,   color},

        {x_pos + char_w, y_pos,          x_uv_to,   y_uv_to,   color},
        {x_pos,          y_pos + char_h, x_uv_from, y_uv_from, color},
        {x_pos + char_w, y_pos + char_h, x_uv_to,   y_uv_from, color},
    };

    *x = *x + glyph->advance;
    memcpy(buffer, vertices, sizeof(vertices));
}

typedef struct File {
    char *buffer;
    size_t size;
} File;

static File read_entire_file(const char *path) {
    File file;
    file.buffer = NULL;
    file.size = 0;

    FILE *handle = fopen(path, "rb");
    if (!handle) {
        printf("ERROR: Could not open file \"%s\".\n", path);
        return file;
    }

    fseek(handle, 0L, SEEK_END);
    size_t file_size = ftell(handle);
    rewind(handle);

    file.buffer = malloc(sizeof(char) * file_size + 1);
    if (!file.buffer) {
        printf("ERROR: Not enough memory to read \"%s\".\n", path);
        fclose(handle);
        return file;
    }
    file.size = file_size;

    size_t bytes_read = fread(file.buffer, sizeof(char), file_size, handle);
    if (bytes_read < file_size) {
        printf("ERROR: Could not read file \"%s\".\n", path);
        free(file.buffer);
        file.buffer = NULL;
        fclose(handle);
        return file;
    }

    file.buffer[bytes_read] = '\0';
    fclose(handle);

    return file;
}

static int window_width = 800;
static int window_height = 600;
static int scroll = 0;
static int cursor = 0;
static bool ctrl_down = false;

LRESULT CALLBACK main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
    case WM_MOUSEWHEEL:
        scroll -= GET_WHEEL_DELTA_WPARAM(w_param);
        break;
    case WM_KEYDOWN:
        if (w_param == VK_RIGHT) cursor += 1;
        if (w_param == VK_LEFT) cursor -= 1;
        if (w_param == VK_CONTROL) ctrl_down = true;
        if (w_param == 'F' && ctrl_down) cursor += 1;
        if (w_param == 'B' && ctrl_down) cursor -= 1;
        break;
    case WM_KEYUP:
        if (w_param == VK_CONTROL) ctrl_down = false;
        break;
    case WM_SIZE:
        window_width = LOWORD(l_param);
        window_height = HIWORD(l_param);
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        window_should_close = true;
        break;
    default:
        result = DefWindowProcA(window, message, w_param, l_param);
        break;
    }
    return result;
}

int main(int argc, const char **argv) {
    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = main_window_callback;
    window_class.hInstance = GetModuleHandle(NULL);
    window_class.lpszClassName = "MainWindow";

    if (!RegisterClassA(&window_class)) {
        printf("unable to register window class\n");
        return 1;
    }

    HWND window = CreateWindowExA(0, window_class.lpszClassName, "Hello, World!", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height, 0, 0, window_class.hInstance, 0);
    if (!window) {
        printf("unable to create window\n");
        return 2;
    }

    HDC window_dc = GetDC(window);
    HGLRC opengl_rc = win32_init_opengl(window_dc);

    wglSwapIntervalEXT(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);
    //glBlendEquation(GL_FUNC_ADD);
    //glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    opengl_compile_shaders();

    FontAtlas atlas = create_font_texture();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texture_location, 0);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    File file = read_entire_file("main.c");
    size_t buffer_size = sizeof(Vertex) * 6 * file.size;
    Vertex *vertices = malloc(buffer_size);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)8);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)16);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    while (!window_should_close) {
        MSG message;
        while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }

        glViewport(0, 0, window_width, window_height);

        float w = 2.0f / window_width;
        float h = 2.0f / window_height;

        float projection[] = {
             w,  0,  0,  0,
             0,  h,  0,  0,
             0,  0,  1,  0,
            -1, -1,  0,  1,
        };

        glUniformMatrix4fv(projection_location, 1, GL_FALSE, projection);

        glClearColor(24.0f/255.0f, 24.0f/255.0f, 24.0f/255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float margin = 5;
        float x = margin;
        float y = window_height + scroll;

        int index = 0;
        for (size_t i = 0; i < file.size; i++) {
            char c = file.buffer[i];
            if (c == '\0') break;
            if (c == '\r') continue;
            if (c == '\t') continue;
            if (c == '\n') {
                y -= atlas.font_height;
                x = margin;
                continue;
            }

            if (i == cursor) {
                float _x = x;
                render_cursor(atlas, &x, y, &vertices[index]);
                x = _x;
                index += 6;
                render_char(atlas, &x, y, c, &vertices[index], BLACK);
            } else {
                render_char(atlas, &x, y, c, &vertices[index], WHITE);
            }

            index += 6;
        }

        glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, buffer_size / (sizeof(float) * 4 + sizeof(uint32_t)));

        SwapBuffers(window_dc);
    }

    return 0;
}
