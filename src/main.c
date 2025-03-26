#include "common.h"
#include "editor.h"
#include "renderer.h"
#include "win32.h"

int main(int argc, const char **argv) {
    // TODO(marla): unused.
    (void)argc;
    (void)argv;

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

    return 0;
}
