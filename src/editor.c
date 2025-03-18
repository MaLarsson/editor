#include "editor.h"

void editor_move_cursor_up(Editor *editor) {
    printf("move up one line!\n");
    (void)editor;
    // TODO(marla)
}

void editor_move_cursor_down(Editor *editor) {
    File *file = &editor->files.data[0];

    int temp_cursor = editor->cursor;
    while (temp_cursor > 0 && file->buffer[temp_cursor - 1] != '\n') {
        temp_cursor -= 1;
    }
    int offset = editor->cursor - temp_cursor;

    while (file->buffer[editor->cursor] != '\n') {
        editor->cursor += 1;
    }

    // TODO(marla): handle if this line is shorter than offset.
    editor->cursor += offset + 1;
}

void editor_move_cursor_forward(Editor *editor) {
    File *file = &editor->files.data[0];
    if (file->buffer[editor->cursor] == '\r') {
        // skip past the \n.
        editor->cursor += 1;
    }
    editor->cursor += 1;
}

void editor_move_cursor_forward_word(Editor *editor) {
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
    File *file = &editor->files.data[0];
    editor->cursor = max(0, editor->cursor - 1);
    if (file->buffer[editor->cursor] == '\r') {
        // skip past the \r.
        editor->cursor = max(0, editor->cursor - 1);
    }
}

void editor_move_cursor_backward_word(Editor *editor) {
    File *file = &editor->files.data[0];
    editor->cursor = max(0, editor->cursor - 1);
    while (!isalnum(file->buffer[editor->cursor]) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
    while (isalnum(file->buffer[editor->cursor - 1]) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
}
