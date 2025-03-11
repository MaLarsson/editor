#include "common.h"
#include "win32.h"
#include "renderer.h"
#include "font.h"

static size_t glyph_index(char c) {
    size_t index = c - ' ';
    // TODO(marla): return the glyph index for box?
    assert(index < GLYPH_COUNT && "invalid glyph index");
    return index;
}

#define RED   0xFF0000FF
#define GREEN 0x00FF00FF
#define BLUE  0x0000FFFF
#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF

static void render_cursor(FontAtlas *atlas, float *x, float y, Vertex *buffer) {
    float line_gap = atlas->font_height - (atlas->ascent - atlas->descent);
    float h = atlas->font_height - line_gap;
    float w = atlas->max_advance;
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

    *x = *x + atlas->max_advance;
    memcpy(buffer, vertices, sizeof(vertices));
}

static void render_char(FontAtlas *atlas, float *x, float y, char c, Vertex *buffer, uint32_t color) {
    int index = glyph_index(c);
    Glyph *glyph = &atlas->glyphs[index];
    float padding = (atlas->font_height - atlas->ascent + atlas->descent) / 2;

    float char_h = glyph->height;
    float char_w = glyph->width;
    float x_pos = *x + glyph->x_bearing;
    float y_pos = y - padding - atlas->ascent - (char_h - glyph->y_bearing);

    float x_uv_from = glyph->x / atlas->texture_width;
    float x_uv_to = (glyph->x + glyph->width) / atlas->texture_width;
    float y_uv_from = glyph->y / atlas->texture_height;
    float y_uv_to = (glyph->y + glyph->height) / atlas->texture_height;

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

    Renderer renderer = {0};
    renderer_init(&renderer);

    FontAtlas atlas = {0};
    font_atlas_init(&atlas);

    File file = read_entire_file("src/main.c");
    size_t buffer_size = sizeof(Vertex) * 6 * file.size;
    Vertex *vertices = malloc(buffer_size);

    while (!window.should_close) {
        win32_poll_events();

        uint32_t bg_color = 0x242424FF;
        uint32_t text_color = WHITE;

        renderer_reset_draw_data(&renderer);
        renderer_update_screen_size(&renderer, window.width, window.height);
        renderer_clear_screen(bg_color);

#if 1
        float margin = 5;
        float x = margin;
        float y = window.height + window.scroll;

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

            if (i == window.cursor) {
                float _x = x;
                render_cursor(&atlas, &x, y, &vertices[index]);
                x = _x;
                index += 6;
                render_char(&atlas, &x, y, c, &vertices[index], bg_color);
            } else {
                render_char(&atlas, &x, y, c, &vertices[index], text_color);
            }

            index += 6;
        }

        glBufferData(GL_ARRAY_BUFFER, buffer_size, vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, buffer_size / (sizeof(float) * 4 + sizeof(uint32_t)));
#else
        render_quad(&renderer, 10, 600, 200, 200, GREEN);
        renderer_draw(&renderer);
#endif

        win32_swap_buffers(&window);
    }

    return 0;
}
