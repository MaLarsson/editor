#include "common.h"
#include "opengl.h"
#include "win32.h"

// TODO(marla): remove gl/gl.h and move free_type into font.c
#include <gl/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static bool window_should_close = false;

WGLSwapIntervalExtProc wglSwapIntervalEXT;
WGLCreateContextAttribsARBProc wglCreateContextAttribsARB;
WGLChoosePixelFormatARBProc wglChoosePixelFormatARB;

static const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in vec4 color;\n"
    "\n"
    "uniform mat4 projection;\n"
    "\n"
    "out vec4 out_color;\n"
    "out vec2 out_uv;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(pos, 0.0, 1.0);\n"
    "    out_uv = uv;\n"
    "    out_color = color;\n"
    "}\n";

static const char *basic_fragment_shader =
    "#version 330 core\n"
    "in vec4 out_color;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = out_color;\n"
    "}\n";

static const char *text_fragment_shader =
    "#version 330 core\n"
    "in vec4 out_color;\n"
    "in vec2 out_uv;\n"
    "\n"
    "uniform sampler2D atlas;\n"
    "\n"
    "void main() {\n"
    "    float d = texture(atlas, out_uv).r;\n"
    "    float aaf = fwidth(d);\n"
    "    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);\n"
    "    gl_FragColor = vec4(out_color.rgb, alpha);\n"
    "}\n";

static GLint projection_location = -1;
static GLint texture_location = -1;
static GLuint text_pipeline = -1;
static GLuint basic_pipeline = -1;

static void opengl_compile_shaders(void) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader, NULL);
    glCompileShader(vertex);

    GLuint basic_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(basic_shader, 1, &basic_fragment_shader, NULL);
    glCompileShader(basic_shader);

    GLuint text_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(text_shader, 1, &text_fragment_shader, NULL);
    glCompileShader(text_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, text_shader);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(basic_shader);
    glDeleteShader(text_shader);

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

    /*
    uint32_t pt_size = 18;
    FT_Size_RequestRec size = {0};
    size.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
    size.height = (pt_size << 6);
    FT_Request_Size(face, &size);
    */
    FT_Set_Pixel_Sizes(face, 0, 18);
    atlas.font_height = face->size->metrics.height / 64.0f;
    atlas.max_advance = face->size->metrics.max_advance / 64.0f;
    atlas.ascent = face->size->metrics.ascender / 64.0f;
    atlas.descent = face->size->metrics.descender / 64.0f;

    atlas.texture_width = 0;
    atlas.texture_height = 0;

    // FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT
    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

    for (char c = ' '; c < '~'; c++) {
        FT_Load_Char(face, c, load_flags);

        atlas.texture_width += face->glyph->bitmap.width;
        GLsizei h = face->glyph->bitmap.rows;
        if (h > atlas.texture_height) atlas.texture_height = h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas.texture_width, atlas.texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    GLsizei x_offset = 0;

    /*
    uint8_t white_buffer[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 2, 2, GL_RED, GL_UNSIGNED_BYTE, white_buffer);
    x_offset += 2;
    */

    for (char c = ' '; c < '~'; c++) {
        if (FT_Load_Char(face, c, load_flags)) {
            printf("ERORR\n");
        }

        size_t index = glyph_index(c);
        Glyph *glyph = &atlas.glyphs[index];
        glyph->x = x_offset;
        glyph->y = 0;
        glyph->x_bearing = face->glyph->bitmap_left;
        glyph->y_bearing = face->glyph->bitmap_top;
        glyph->advance = (face->glyph->advance.x >> 6);
        glyph->width = face->glyph->bitmap.width;
        glyph->height = face->glyph->bitmap.rows;

        if (c == '!') {

        }

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

static int window_width = 1200;
static int window_height = 1200;
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
    win32_swap_interval(1);

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    opengl_compile_shaders();

    FontAtlas atlas = create_font_texture();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(texture_location, 0);

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    File file = read_entire_file("src/main.c");
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
