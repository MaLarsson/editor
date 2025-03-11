#include "font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

static size_t glyph_index(char c) {
    size_t index = c - ' ';
    // TODO(marla): return the glyph index for box?
    assert(index < GLYPH_COUNT && "invalid glyph index");
    return index;
}

void font_atlas_init(FontAtlas *atlas) {
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &atlas->texture);
    glBindTexture(GL_TEXTURE_2D, atlas->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        printf("error loading freetype library\n");
    }

    FT_Face face;
    if (FT_New_Face(library, "C:/Windows/Fonts/consola.ttf", 0, &face)) {
        printf("error loading font\n");
    }

    FT_Set_Pixel_Sizes(face, 0, 18);
    atlas->font_height = face->size->metrics.height / 64.0f;
    atlas->max_advance = face->size->metrics.max_advance / 64.0f;
    atlas->ascent = face->size->metrics.ascender / 64.0f;
    atlas->descent = face->size->metrics.descender / 64.0f;

    atlas->texture_width = 0;
    atlas->texture_height = 0;

    FT_Int32 load_flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;

    for (char c = ' '; c < '~'; c++) {
        if (FT_Load_Char(face, c, load_flags)) {
            printf("ERROR: unable to load char\n");
        }

        atlas->texture_width += face->glyph->bitmap.width;
        GLsizei h = face->glyph->bitmap.rows;
        if (h > atlas->texture_height) atlas->texture_height = h;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->texture_width, atlas->texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    GLsizei x_offset = 0;

    for (char c = ' '; c < '~'; c++) {
        if (FT_Load_Char(face, c, load_flags)) {
            printf("ERROR: unable to load char\n");
        }

        size_t index = glyph_index(c);
        Glyph *glyph = &atlas->glyphs[index];
        glyph->x = x_offset;
        glyph->y = 0;
        glyph->x_bearing = face->glyph->bitmap_left;
        glyph->y_bearing = face->glyph->bitmap_top;
        glyph->advance = (face->glyph->advance.x >> 6);
        glyph->width = face->glyph->bitmap.width;
        glyph->height = face->glyph->bitmap.rows;

        x_offset += glyph->width;
        glTexSubImage2D(GL_TEXTURE_2D, 0, glyph->x, glyph->y, glyph->width, glyph->height, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}
