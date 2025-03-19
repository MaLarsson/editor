#ifndef EDITOR_H_
#define EDITOR_H_

#include "common.h"

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

    // TODO(marla): these should be per file?
    int cursor;
    int scroll;
    int vertical_move_offset_cache;
} Editor;

void editor_move_cursor_up(Editor *editor);
void editor_move_cursor_down(Editor *editor);
void editor_move_cursor_forward(Editor *editor);
void editor_move_cursor_forward_word(Editor *editor);
void editor_move_cursor_backward(Editor *editor);
void editor_move_cursor_backward_word(Editor *editor);
void editor_move_cursor_start_of_line(Editor *editor);
void editor_move_cursor_end_of_line(Editor *editor);

void editor_type_char(Editor *editor, char c);
void editor_backspace(Editor *editor);
void editor_backspace_word(Editor *editor);
void editor_delete(Editor *editor);

void editor_scroll_up(Editor *editor);
void editor_scroll_down(Editor *editor);

#endif // EDITOR_H_
