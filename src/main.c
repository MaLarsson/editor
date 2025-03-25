#include "common.h"
#include "editor.h"
#include "renderer.h"
#include "win32.h"

typedef struct {
    char *data;
    size_t count;
    size_t capacity;
    size_t gap;
} GapBuffer;

static size_t gap_buffer_gap_size(GapBuffer *buffer) {
    return buffer->capacity - buffer->count;
}

static void gap_buffer_realloc(GapBuffer *buffer) {
    size_t gap_size = gap_buffer_gap_size(buffer);
    buffer->capacity *= 2;
    buffer->data = realloc(buffer->data, buffer->capacity);

    size_t tail_length = buffer->count - buffer->gap;
    char *tail_start = &buffer->data[buffer->gap + gap_size];
    char *new_tail_start = &buffer->data[buffer->gap + gap_buffer_gap_size(buffer)];

    memmove(tail_start, new_tail_start, tail_length);
}

static void gap_buffer_dump(GapBuffer *buffer) {
    printf("count: %llu, capacity: %llu\n", buffer->count, buffer->capacity);
}

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    GapBuffer buffer = {0};
    gap_buffer_dump(&buffer);

#if 0
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
        win32_poll_events(&window, &editor);

        renderer_reset_draw_data(&renderer);
        renderer_update_screen_size(&renderer, window.width, window.height);
        renderer_clear_screen(editor.theme.background_color);

        float margin = 5;
        float x = margin;
        float y = (float)window.height + (float)editor.scroll;
        int tab_width = 4;

        File *file = &editor.files.data[0];

        // render quads.
        // cursor, hightlightning, file info bar, etc.
        for (size_t i = 0; i < file->size; ++i) {
            if (i == editor.cursor) {
                float line_gap = atlas.font_height - (atlas.ascent - atlas.descent);
                float h = atlas.font_height - line_gap;
                float w = atlas.max_advance;
                render_quad(&renderer, x, y - h, w, h, editor.theme.cursor_color);
                break;
            }

            char c = file->buffer[i];

            switch (c) {
            case ' ': {
                x += atlas.max_advance;
            } break;
            case '\r': {
                // do nothing.
            } break;
            case '\n': {
                x = margin;
                y -= atlas.font_height;
            } break;
            case '\t': {
                x += atlas.max_advance * tab_width;
            } break;
            default: {
                x += atlas.glyphs[glyph_index(c)].advance;
            } break;
            }
        }

        x = margin;
        y = (float)window.height + (float)editor.scroll;

        // render all the characters.
        for (size_t i = 0; i < file->size; ++i) {
            char c = file->buffer[i];

            switch (c) {
            case ' ': {
                x += atlas.max_advance;
            } break;
            case '\r': {
                // do nothing.
            } break;
            case '\n': {
                x = margin;
                y -= atlas.font_height;
            } break;
            case '\t': {
                x += atlas.max_advance * tab_width;
            } break;
            default: {
                // tokenize.
                size_t start = i;
                while (file->buffer[i + 1] != '\0' && !isspace(file->buffer[i + 1])) {
                    ++i;
                }

                uint32_t text_color = editor.theme.text_color;
                uint32_t bg_color = editor.theme.background_color;

                if (start <= editor.cursor && i >= editor.cursor) {
                    StringView prefix = {&file->buffer[start], editor.cursor - start};
                    StringView suffix = {&file->buffer[editor.cursor + 1], i - editor.cursor};
                    x += render_text(&renderer, &atlas, x, y, prefix, text_color);
                    x += render_glyph(&renderer, &atlas, x, y, file->buffer[editor.cursor], bg_color);
                    x += render_text(&renderer, &atlas, x, y, suffix, text_color);
                } else {
                    StringView text = {&file->buffer[start], i - start + 1};
                    x += render_text(&renderer, &atlas, x, y, text, text_color);
                }
            } break;
            }
        }

        renderer_draw(&renderer);
        win32_swap_buffers(&window);
    }
#endif

    return 0;
}
