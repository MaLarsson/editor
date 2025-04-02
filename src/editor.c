#include "editor.h"
#include "win32.h"

void editor_render_file(Editor *editor, int width, int height, Renderer *renderer) {
    UNUSED(width);

    float margin = 5;
    float x = margin;
    float y = (float)height + (float)editor->scroll;
    int tab_width = 4;

    GapBuffer *buffer = &editor->buffer;

    // render quads.
    // cursor, hightlightning, file info bar, etc.
    for (size_t i = 0; i < buffer->count; ++i) {
        if (i == editor->cursor) {
            float line_gap = editor->font.font_height - (editor->font.ascent - editor->font.descent);
            float h = editor->font.font_height - line_gap;
            float w = editor->font.max_advance;
            render_quad(renderer, x, y - h, w, h, editor->theme.cursor_color);
            break;
        }

        char c = gap_buffer_at(buffer, i);

        switch (c) {
        case ' ': {
            x += editor->font.max_advance;
        } break;
        case '\r': {
            // do nothing.
        } break;
        case '\n': {
            x = margin;
            y -= editor->font.font_height;
        } break;
        case '\t': {
            x += editor->font.max_advance * tab_width;
        } break;
        default: {
            x += editor->font.glyphs[glyph_index(c)].advance;
        } break;
        }
    }

    x = margin;
    y = (float)height + (float)editor->scroll;

    // render all the characters.
    for (size_t i = 0; i < buffer->count; ++i) {
        char c = gap_buffer_at(buffer, i);

        switch (c) {
        case ' ': {
            x += editor->font.max_advance;
        } break;
        case '\r': {
            // do nothing.
        } break;
        case '\n': {
            x = margin;
            y -= editor->font.font_height;
        } break;
        case '\t': {
            x += editor->font.max_advance * tab_width;
        } break;
        default: {
            // tokenize.
            //size_t start = i;
            while (gap_buffer_at(buffer, i + 1) != '\0' && !isspace(gap_buffer_at(buffer, i + 1))) {
                ++i;
            }

            //uint32_t text_color = editor->theme.text_color;
            //uint32_t bg_color = editor->theme.background_color;

            /*
              TODO(marla): get a string view into the gap buffer unless we're on the gap line, then we can use temp storage.
            if (start <= editor->cursor && i >= editor->cursor) {
                StringView prefix = {&file->buffer[start], editor->cursor - start};
                StringView suffix = {&file->buffer[editor->cursor + 1], i - editor->cursor};
                x += render_text(renderer, &editor->font, x, y, prefix, text_color);
                x += render_glyph(renderer, &editor->font, x, y, file->buffer[editor->cursor], bg_color);
                x += render_text(renderer, &editor->font, x, y, suffix, text_color);
            } else {
                StringView text = {&file->buffer[start], i - start + 1};
                x += render_text(renderer, &editor->font, x, y, text, text_color);
            }
            */
        } break;
        }
    }
}

void editor_move_cursor_up(Editor *editor) {
    GapBuffer *buffer = &editor->buffer;

    if (editor->vertical_move_offset_cache == -1) {
        int temp_cursor = editor->cursor;
        while (temp_cursor > 0 && gap_buffer_at(buffer, temp_cursor - 1) != '\n') {
            temp_cursor -= 1;
        }
        editor->vertical_move_offset_cache = editor->cursor - temp_cursor;
    }

    int offset = editor->vertical_move_offset_cache;
    int old_cursor = editor->cursor;

    while (editor->cursor > 0 && gap_buffer_at(buffer, editor->cursor) != '\n') {
        editor->cursor -= 1;
    }

    if (editor->cursor == 0) {
        // cursor is already on first line, dont move.
        editor->cursor = old_cursor;
        return;
    }

    editor->cursor -= 1;
    while (gap_buffer_at(buffer, editor->cursor) != '\n') {
        editor->cursor -= 1;
        if (editor->cursor == -1) break;
    }
    editor->cursor += 1;

    for (int i = 0; i < offset; ++i) {
        char c = gap_buffer_at(buffer, editor->cursor);
        if (c == '\r' || c == '\n') break;
        editor->cursor += 1;
    }
}

void editor_move_cursor_down(Editor *editor) {
    GapBuffer *buffer = &editor->buffer;

    if (editor->vertical_move_offset_cache == -1) {
        int temp_cursor = editor->cursor;
        while (temp_cursor > 0 && gap_buffer_at(buffer, temp_cursor - 1) != '\n') {
            temp_cursor -= 1;
        }
        editor->vertical_move_offset_cache = editor->cursor - temp_cursor;
    }

    int offset = editor->vertical_move_offset_cache;

    // TODO(marla): handle end of file.
    while (gap_buffer_at(buffer, editor->cursor) != '\n') {
        editor->cursor += 1;
    }
    editor->cursor += 1;

    for (int i = 0; i < offset; ++i) {
        char c = gap_buffer_at(buffer, editor->cursor);
        if (c == '\r' || c == '\n') break;
        editor->cursor += 1;
    }
}

void editor_move_cursor_forward(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    if (gap_buffer_at(buffer, editor->cursor) == '\r') {
        // skip past the \n.
        editor->cursor += 1;
    }
    editor->cursor += 1;
}

void editor_move_cursor_forward_word(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    // TODO(marla): handle end of file.
    while (!isalnum(gap_buffer_at(buffer, editor->cursor))) {
        editor->cursor += 1;
    }
    // TODO(marla): handle end of file.
    while (isalnum(gap_buffer_at(buffer, editor->cursor))) {
        editor->cursor += 1;
    }
}

void editor_move_cursor_backward(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    editor->cursor = max(0, editor->cursor - 1);
    if (gap_buffer_at(buffer, editor->cursor) == '\r') {
        // skip past the \r.
        editor->cursor = max(0, editor->cursor - 1);
    }
}

void editor_move_cursor_backward_word(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    editor->cursor = max(0, editor->cursor - 1);
    while (!isalnum(gap_buffer_at(buffer, editor->cursor)) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
    while (isalnum(gap_buffer_at(buffer, editor->cursor - 1)) && editor->cursor > 0) {
        editor->cursor -= 1;
    }
}

void editor_move_cursor_start_of_line(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    while (editor->cursor > 0 && gap_buffer_at(buffer, editor->cursor - 1) != '\n') {
        editor->cursor -= 1;
    }
}

void editor_move_cursor_end_of_line(Editor *editor) {
    editor->vertical_move_offset_cache = -1;

    GapBuffer *buffer = &editor->buffer;
    // TODO(marla): handle end of file.
    while (gap_buffer_at(buffer, editor->cursor) != '\r' && gap_buffer_at(buffer, editor->cursor) != '\n') {
        editor->cursor += 1;
    }
}

void editor_type_char(Editor *editor, char c) {
    gap_buffer_insert_char(&editor->buffer, editor->cursor++, c);
}

void editor_backspace(Editor *editor) {
    UNUSED(editor);
}

void editor_backspace_word(Editor *editor) {
    UNUSED(editor);
}

void editor_delete(Editor *editor) {
    UNUSED(editor);
}

void editor_scroll_up(Editor *editor) {
    // TODO(marla): scroll exactly one line.
    editor->scroll = max(0, editor->scroll - 21);
}

void editor_scroll_down(Editor *editor) {
    // TODO(marla): scroll exactly one line.
    editor->scroll += 21;
}
