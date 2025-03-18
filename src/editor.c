#include "editor.h"

void editor_move_cursor_up(Editor *editor) {
    printf("move up one line!\n");
    (void)editor;
    // TODO(marla)
}

void editor_move_cursor_down(Editor *editor) {
    File *file = &editor->files.data[0];

    int offset = 1;
    while (editor->cursor - offset > 0 && file->buffer[editor->cursor - offset - 1] != '\n') {
        offset += 1;
    }

    printf("%d\n", offset);
    printf("move down one line!\n");

    // TODO(marla)
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
