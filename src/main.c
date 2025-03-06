#include "common.h"
#include "opengl.h"
#include "win32.h"

// TODO(marla): remove windows.h and gl/gl.h and move free_type into font.c
#include <windows.h>
#include <gl/gl.h>
#include <ft2build.h>
#include FT_FREETYPE_H

static bool window_should_close = false;

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

static void create_font_atlas(FontAtlas *atlas) {
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
    atlas->font_height = face->size->metrics.height / 64.0f;
    atlas->max_advance = face->size->metrics.max_advance / 64.0f;
    atlas->ascent = face->size->metrics.ascender / 64.0f;
    atlas->descent = face->size->metrics.descender / 64.0f;

    atlas->texture_width = 0;
    atlas->texture_height = 0;

    // FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT
    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);

    for (char c = ' '; c < '~'; c++) {
        FT_Load_Char(face, c, load_flags);

        atlas->texture_width += face->glyph->bitmap.width;
        GLsizei h = face->glyph->bitmap.rows;
        if (h > atlas->texture_height) atlas->texture_height = h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->texture_width, atlas->texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

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
        Glyph *glyph = &atlas->glyphs[index];
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

int main(int argc, const char **argv) {
    Window window = {0};
    win32_init_window(&window, 1200, 1200, "Editor");
    win32_swap_interval(1);

    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    opengl_compile_shaders();

    FontAtlas atlas = {0};
    create_font_atlas(&atlas);

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

    while (!window.should_close) {
        win32_poll_events();

        glViewport(0, 0, 1200 /*window->width*/, 1200 /*window->height*/);

        float w = 2.0f / 1200; //window->width;
        float h = 2.0f / 1200; //window->height;

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
        float y = 1200 /*window->height*/ + 0; // scroll;

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

            if (i == /*cursor*/0) {
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

        win32_swap_buffers(&window);
        //SwapBuffers(window_dc);
    }

    return 0;
}
