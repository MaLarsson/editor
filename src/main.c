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

static void gap_buffer_grow(GapBuffer *buffer, size_t size) {
    UNUSED(size);

    size_t gap_size = gap_buffer_gap_size(buffer);
    buffer->capacity *= 2;
    buffer->data = realloc(buffer->data, buffer->capacity);

    size_t tail_length = buffer->count - buffer->gap;
    char *tail_start = &buffer->data[buffer->gap + gap_size];
    char *new_tail_start = &buffer->data[buffer->gap + gap_buffer_gap_size(buffer)];

    memmove(tail_start, new_tail_start, tail_length);
}

static void gap_buffer_move_gap(GapBuffer *buffer, size_t index) {
    UNUSED(buffer);
    UNUSED(index);
    // TODO(marla): actually move the gap.
}

void gap_buffer_insert_string(GapBuffer *buffer, size_t index, const char *string) {
    if (index != buffer->gap) {
        gap_buffer_move_gap(buffer, index);
    }
    size_t len = strlen(string);
    memcpy(&buffer->data[buffer->gap], string, len);
    buffer->gap += len;
    buffer->count += len;
}

void gap_buffer_insert_char(GapBuffer *buffer, size_t index, char c) {
    if (index != buffer->gap) {
        gap_buffer_move_gap(buffer, index);
    }
    buffer->data[buffer->gap++] = c;
    buffer->count += 1;
}

static void gap_buffer_dump(GapBuffer *buffer) {
    printf("count: %llu, capacity: %llu\n", buffer->count, buffer->capacity);
    printf("data: \"");

    for (size_t i = 0; i < buffer->gap; ++i) {
        printf("%c", buffer->data[i]);
    }

    printf("[");
    size_t gap_size = gap_buffer_gap_size(buffer);
    for (size_t i = 0; i < gap_size; ++i) {
        printf(" ");
    }
    printf("]");

    for (size_t i = buffer->gap + gap_size; i < buffer->capacity; ++i) {
        printf("%c", buffer->data[i]);
    }

    printf("\"\n");
}

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    GapBuffer buffer = {0};
    gap_buffer_dump(&buffer);

    buffer.data = malloc(sizeof(char) * 32);
    buffer.count = 0;
    buffer.capacity = 32;
    buffer.gap = 0;
    gap_buffer_dump(&buffer);

    gap_buffer_insert_string(&buffer, 0, "hello world");
    gap_buffer_dump(&buffer);

    gap_buffer_insert_char(&buffer, buffer.gap, '!');
    gap_buffer_insert_char(&buffer, buffer.gap, '~');
    gap_buffer_insert_char(&buffer, buffer.gap, '~');
    gap_buffer_dump(&buffer);

    // dealloc.
    free(buffer.data);

#if 0
    Window window = {0};
    win32_init_window(&window, 1200, 1200, "Editor");
    win32_swap_interval(1);

    Renderer renderer = {0};
    renderer_init(&renderer);

    Editor editor = {0};
    renderer_init_font_atlas(&editor.font);
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

        editor_render_file(&editor, window.width, window.height, &renderer);

        renderer_draw(&renderer);
        win32_swap_buffers(&window);
    }
#endif

    return 0;
}
