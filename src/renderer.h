#ifndef RENDERER_H_
#define RENDERER_H_

#include "common.h"
#include "opengl.h"

#define RED   0xFF0000FF
#define GREEN 0x00FF00FF
#define BLUE  0x0000FFFF
#define WHITE 0xFFFFFFFF
#define BLACK 0x000000FF

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

void renderer_init_font_atlas(FontAtlas *atlas);
size_t glyph_index(char c);

typedef struct {
    Vertex *data;
    uint32_t count;
    uint32_t capacity;
} Vertices;

typedef struct {
    uint32_t *data;
    uint32_t count;
    uint32_t capacity;
} Indices;

typedef struct {
    uint32_t index_offset;
    uint32_t elements;
    GLuint program;
} DrawCall;

typedef struct {
    DrawCall *data;
    uint32_t count;
    uint32_t capacity;
} DrawCalls;

typedef struct {
    GLuint text_program;
    GLuint basic_program;
    GLuint vao;
    GLuint vbo;

    Vertices vertices;
    Indices indices;
    DrawCalls draw_calls;
} Renderer;

void renderer_init(Renderer *renderer);
void renderer_reset_draw_data(Renderer *renderer);
void renderer_update_screen_size(Renderer *renderer, int width, int height);
void renderer_clear_screen(uint32_t color);
void renderer_draw(Renderer *renderer);

void renderer_begin_draw_call(Renderer *renderer, GLuint shader);
void renderer_end_draw_call(Renderer *renderer);

void render_quad(Renderer *renderer, float x, float y, float w, float h, uint32_t color);
float render_glyph(Renderer *renderer, FontAtlas *font, float x, float y, char c, uint32_t color);
float render_text(Renderer *renderer, FontAtlas *font, float x, float y, const char *text, uint32_t color);

#endif // RENDERER_H_
