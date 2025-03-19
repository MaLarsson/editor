#include "editor.h"

void editor_move_cursor_up(Editor *editor) {
    File *file = &editor->files.data[0];

    if (editor->vertical_move_offset_cache == -1) {
        int temp_cursor = editor->cursor;
        while (temp_cursor > 0 && file->buffer[temp_cursor - 1] != '\n') {
            temp_cursor -= 1;
        }
        editor->vertical_move_offset_cache = editor->cursor - temp_cursor;
    }

    int offset = editor->vertical_move_offset_cache;

    // TODO(marla): handle beginning of file.
    while (file->buffer[editor->cursor] != '\n') {
        editor->cursor -= 1;
    }
    editor->cursor -= 1;
    while (file->buffer[editor->cursor] != '\n') {
        editor->cursor -= 1;
    }
    editor->cursor += 1;

    for (int i = 0; i < offset; ++i) {
        char c = file->buffer[editor->cursor];
        if (c == '\r' || c == '\n') break;
        editor->cursor += 1;
    }
}

void editor_move_cursor_down(Editor *editor) {
    File *file = &editor->files.data[0];

    if (editor->vertical_move_offset_cache == -1) {
        int temp_cursor = editor->cursor;
        while (temp_cursor > 0 && file->buffer[temp_cursor - 1] != '\n') {
            temp_cursor -= 1;
        }
        editor->vertical_move_offset_cache = editor->cursor - temp_cursor;
    }

    int offset = editor->vertical_move_offset_cache;

    // TODO(marla): handle end of file.
    while (file->buffer[editor->cursor] != '\n') {
        editor->cursor += 1;
    }
    editor->cursor += 1;

    for (int i = 0; i < offset; ++i) {
        char c = file->buffer[editor->cursor];
        if (c == '\r' || c == '\n') break;
        editor->cursor += 1;
    }
}

void editor_move_cursor_forward(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];
    if (file->buffer[editor->cursor] == '\r') {
        // skip past the \n.
        editor->cursor += 1;
    }
    editor->cursor += 1;
}

void editor_move_cursor_forward_word(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];

    // TODO(marla): handle end of file.
    while (!isalnum(file->buffer[editor->cursor])) {
        editor->cursor += 1;
    }

    // TODO(marla): handle end of file.
    while (isalnum(file->buffer[editor->cursor])) {
        editor->cursor += 1;
    }
}

void editor_move_cursor_backward(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];
    editor->cursor = max(0, editor->cursor - 1);
    if (file->buffer[editor->cursor] == '\r') {
        // skip past the \r.
        editor->cursor = max(0, editor->cursor - 1);
    }
}

void editor_move_cursor_backward_word(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];
    editor->cursor = max(0, editor->cursor - 1);
    while (!isalnum(file->buffer[editor->cursor]) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
    while (isalnum(file->buffer[editor->cursor - 1]) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
}

void editor_move_cursor_start_of_line(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];
    while (editor->cursor > 0 && file->buffer[editor->cursor - 1] != '\n') {
        editor->cursor -= 1;
    }
}

void editor_move_cursor_end_of_line(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    File *file = &editor->files.data[0];
    // TODO(marla): handle end of file.
    while (file->buffer[editor->cursor] != '\r' && file->buffer[editor->cursor] != '\n') {
        editor->cursor += 1;
    }
}
