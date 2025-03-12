#include "common.h"
#include "win32.h"
#include "renderer.h"

typedef struct {
    File *data;
    uint32_t count;
    uint32_t capacity;
} Files;

typedef struct {
    uint32_t background_color;
    uint32_t text_color;
    uint32_t highlight_color;
    uint32_t cursor_color;
} Theme;

typedef struct {
    Files files;
    Theme theme;
} Editor;

int main(int argc, const char **argv) {
    Window window = {0};
    win32_init_window(&window, 1200, 1200, "Editor");
    win32_swap_interval(1);

    Renderer renderer = {0};
    renderer_init(&renderer);

    FontAtlas atlas = {0};
    renderer_init_font_atlas(&atlas);

    Editor editor = {0};
    editor.theme.background_color = 0x242424FF;
    editor.theme.text_color = WHITE;
    editor.theme.highlight_color = 0x606060FF;
    editor.theme.cursor_color = GREEN;
    array_add(&editor.files, read_entire_file("src/main.c"));

    while (!window.should_close) {
        win32_poll_events();

        renderer_reset_draw_data(&renderer);
        renderer_update_screen_size(&renderer, window.width, window.height);
        renderer_clear_screen(editor.theme.background_color);

        float margin = 5;
        float x = margin;
        float y = window.height + window.scroll;

        File *file = &editor.files.data[0];

        const char *start = file->buffer + 1;
        bool carriage_return = false;

        x += render_glyph(&renderer, &atlas, x, y, file->buffer[0], editor.theme.background_color);

        for (size_t i = 1; i < file->size; ++i) {
            char c = file->buffer[i];
            if (c == '\r' || c == '\n') {
                const char *end = &file->buffer[i];
                size_t len = end - start;
                if (len > 0 && !carriage_return) {
                    StringView text = {start, len};
                    render_text(&renderer, &atlas, x, y, text, editor.theme.text_color);
                    x = margin;
                    y -= atlas.font_height;
                }
                carriage_return = (c == '\r');
                start = end;
            }
        }

        /*
        const char *text = "hello world this is a long string printed over multiple lines.";

        x += render_glyph(&renderer, &atlas, x, y, text[0], editor.theme.background_color);
        StringView view0 = {text + 1, 10};
        render_text(&renderer, &atlas, x, y, view0, editor.theme.text_color);

        x = margin;
        y -= atlas.font_height;
        StringView view1 = {text + 12, strlen(text) - 12};
        render_text(&renderer, &atlas, x, y, view1, editor.theme.text_color);
        */

        float line_gap = atlas.font_height - (atlas.ascent - atlas.descent);
        float h = atlas.font_height - line_gap;
        float w = atlas.max_advance;
        render_quad(&renderer, margin, window.height + window.scroll - h, w, h, editor.theme.cursor_color);

        renderer_draw(&renderer);
        win32_swap_buffers(&window);
    }

    return 0;
}
