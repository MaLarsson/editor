#include "renderer.h"

#include <ft2build.h>
#include FT_FREETYPE_H

size_t glyph_index(char c) {
    size_t index = c - ' ';
    // TODO(marla): return the glyph index for box?
    assert(index < GLYPH_COUNT && "invalid glyph index");
    return index;
}

void renderer_init_font_atlas(FontAtlas *atlas) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &atlas->texture);
    glBindTexture(GL_TEXTURE_2D, atlas->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        printf("error loading freetype library\n");
    }

    FT_Face face;
    if (FT_New_Face(library, "C:/Windows/Fonts/consola.ttf", 0, &face)) {
        printf("error loading font\n");
    }

    FT_Set_Pixel_Sizes(face, 0, 18);
    atlas->font_height = face->size->metrics.height / 64.0f;
    atlas->max_advance = face->size->metrics.max_advance / 64.0f;
    atlas->ascent = face->size->metrics.ascender / 64.0f;
    atlas->descent = face->size->metrics.descender / 64.0f;

    atlas->texture_width = 0;
    atlas->texture_height = 0;

    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;

    for (char c = ' '; c < '~'; ++c) {
        if (FT_Load_Char(face, c, load_flags)) {
            printf("ERROR: unable to load char\n");
        }

        atlas->texture_width += face->glyph->bitmap.width;
        GLsizei h = face->glyph->bitmap.rows;
        if (h > atlas->texture_height) atlas->texture_height = (float)h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei)atlas->texture_width, (GLsizei)atlas->texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    GLsizei x_offset = 0;

    for (char c = ' '; c < '~'; ++c) {
        if (FT_Load_Char(face, c, load_flags)) {
            printf("ERROR: unable to load char\n");
        }

        // TODO(marla): maybe we should not convert from and to float all the time.
        size_t index = glyph_index(c);
        Glyph *glyph = &atlas->glyphs[index];
        glyph->x = (float)x_offset;
        glyph->y = 0;
        glyph->x_bearing = (float)face->glyph->bitmap_left;
        glyph->y_bearing = (float)face->glyph->bitmap_top;
        glyph->advance = (float)(face->glyph->advance.x >> 6);
        glyph->width = (float)face->glyph->bitmap.width;
        glyph->height = (float)face->glyph->bitmap.rows;

        x_offset += (GLsizei)glyph->width;
        glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)glyph->x, (GLint)glyph->y, (GLsizei)glyph->width, (GLsizei)glyph->height,
                        GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}

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
    "    out_color = vec4(color.a, color.b, color.g, color.r);\n"
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
    "    gl_FragColor = vec4(out_color.rgb, texture(atlas, out_uv).r);\n"
    "}\n";

static void compile_shaders(Renderer *renderer) {
    UNUSED(renderer);

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader, NULL);
    glCompileShader(vertex);

    GLuint basic_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(basic_shader, 1, &basic_fragment_shader, NULL);
    glCompileShader(basic_shader);

    GLuint text_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(text_shader, 1, &text_fragment_shader, NULL);
    glCompileShader(text_shader);

    GLuint basic_program = glCreateProgram();
    glAttachShader(basic_program, vertex);
    glAttachShader(basic_program, basic_shader);
    glLinkProgram(basic_program);

    GLuint text_program = glCreateProgram();
    glAttachShader(text_program, vertex);
    glAttachShader(text_program, text_shader);
    glLinkProgram(text_program);

    glDeleteShader(vertex);
    glDeleteShader(basic_shader);
    glDeleteShader(text_shader);

    glUseProgram(text_program);

    renderer->basic_program = basic_program;
    renderer->text_program = text_program;
}

void renderer_init(Renderer *renderer) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    compile_shaders(renderer);

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)8);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)16);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void renderer_reset_draw_data(Renderer *renderer) {
    renderer->vertices.count = 0;
    renderer->indices.count = 0;
    renderer->draw_calls.count = 0;
}

void renderer_update_screen_size(Renderer *renderer, int width, int height) {
    glViewport(0, 0, width, height);

    float w = 2.0f / width;
    float h = 2.0f / height;

    float projection[] = {
         w,  0,  0,  0,
         0,  h,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1,
    };

    glUseProgram(renderer->text_program);
    GLint text_projection = glGetUniformLocation(renderer->text_program, "projection");
    glUniformMatrix4fv(text_projection, 1, GL_FALSE, projection);

    glUseProgram(renderer->basic_program);
    GLint basic_projection = glGetUniformLocation(renderer->basic_program, "projection");
    glUniformMatrix4fv(basic_projection, 1, GL_FALSE, projection);
}

void renderer_clear_screen(uint32_t color) {
    Vec4f background = hex_to_vec4f(color);
    glClearColor(background.x, background.y, background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_draw(Renderer *renderer) {
    size_t buffer_size = renderer->vertices.count * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, renderer->vertices.data, GL_DYNAMIC_DRAW);

    glUseProgram(renderer->basic_program);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(renderer->text_program);
    glDrawArrays(GL_TRIANGLES, 6, renderer->vertices.count - 6);
}

void renderer_begin_draw_call(Renderer *renderer, GLuint shader) {
    UNUSED(renderer);
    UNUSED(shader);
    // ...
}

void renderer_end_draw_call(Renderer *renderer) {
    UNUSED(renderer);
    // ...
}

void render_quad(Renderer *renderer, float x, float y, float w, float h, uint32_t color) {
    Vertex v0 = {x,     y,     0, 1, color};
    Vertex v1 = {x,     y + h, 0, 0, color};
    Vertex v2 = {x + w, y,     1, 1, color};
    Vertex v3 = {x + w, y,     1, 1, color}; // duplicate v2
    Vertex v4 = {x,     y + h, 0, 0, color}; // duplicate v1
    Vertex v5 = {x + w, y + h, 1, 0, color};

    array_add(&renderer->vertices, v0);
    array_add(&renderer->vertices, v1);
    array_add(&renderer->vertices, v2);
    array_add(&renderer->vertices, v3);
    array_add(&renderer->vertices, v4);
    array_add(&renderer->vertices, v5);
}

float render_glyph(Renderer *renderer, FontAtlas *font, float x, float y, char c, uint32_t color) {
    if (c == '\r' || c == '\n' || c == '\t') return 0.0f;

    size_t index = glyph_index(c);
    Glyph *glyph = &font->glyphs[index];
    float padding = (font->font_height - font->ascent + font->descent) / 2;

    float char_h = glyph->height;
    float char_w = glyph->width;
    float x_pos = x + glyph->x_bearing;
    float y_pos = y - padding - font->ascent - (char_h - glyph->y_bearing);

    float x_uv_from = glyph->x / font->texture_width;
    float x_uv_to = (glyph->x + glyph->width) / font->texture_width;
    float y_uv_from = glyph->y / font->texture_height;
    float y_uv_to = (glyph->y + glyph->height) / font->texture_height;

    Vertex v0 = {x_pos,          y_pos,          x_uv_from, y_uv_to,   color};
    Vertex v1 = {x_pos,          y_pos + char_h, x_uv_from, y_uv_from, color};
    Vertex v2 = {x_pos + char_w, y_pos,          x_uv_to,   y_uv_to,   color};
    Vertex v3 = {x_pos + char_w, y_pos,          x_uv_to,   y_uv_to,   color}; // duplicate v2
    Vertex v4 = {x_pos,          y_pos + char_h, x_uv_from, y_uv_from, color}; // duplicate v1
    Vertex v5 = {x_pos + char_w, y_pos + char_h, x_uv_to,   y_uv_from, color};

    array_add(&renderer->vertices, v0);
    array_add(&renderer->vertices, v1);
    array_add(&renderer->vertices, v2);
    array_add(&renderer->vertices, v3);
    array_add(&renderer->vertices, v4);
    array_add(&renderer->vertices, v5);

    return glyph->advance;
}

float render_text(Renderer *renderer, FontAtlas *font, float x, float y, StringView text, uint32_t color) {
    float offset = x;
    for (size_t i = 0; i < text.len; ++i) {
        offset += render_glyph(renderer, font, offset, y, text.data[i], color);
    }
    return offset - x;
}
