#ifndef FONT_H_
#define FONT_H_

#include "common.h"
#include "opengl.h"

typedef struct {
    float x;
    float y;
    float x_bearing;
    float y_bearing;
    float advance;
    float width;
    float height;
} Glyph;

#define GLYPH_COUNT 255

typedef struct {
    float texture_width;
    float texture_height;
    GLuint texture;
    Glyph glyphs[GLYPH_COUNT];
    float font_height;
    float max_advance;
    float ascent;
    float descent;
} FontAtlas;

#endif // FONT_H_
