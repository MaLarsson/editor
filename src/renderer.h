#ifndef RENDERER_H_
#define RENDERER_H_

#include "common.h"
#include "opengl.h"

typedef struct {
    Vertex *data;
    uint32_t count;
    uint32_t capacity;
} Vertices;

typedef struct {
    GLuint text_program;
    GLuint basic_program;
    GLint projection_location;
    GLint texture_location;
    Vertices draw_data;
} Renderer;

void renderer_init(Renderer *renderer);

#endif // RENDERER_H_
