#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define array_add(xs, x)                                                \
    do {                                                                \
        if ((xs)->count >= (xs)->capacity) {                            \
            if ((xs)->capacity == 0) (xs)->capacity = 256;              \
            else (xs)->capacity *= 2;                                   \
            (xs)->data = realloc((xs)->data, (xs)->capacity*sizeof(*(xs)->data)); \
        }                                                               \
                                                                        \
        (xs)->data[(xs)->count++] = (x);                                \
    } while (0)

#define KB(sz) 1024*sz
#define MB(sz) 1024*1024*sz

#define MAX_SCRATCH_BUFFER MB(10)

typedef struct {
    uint8_t data[MAX_SCRATCH_BUFFER];
    uint32_t len;
} Scratch;

typedef struct {
    char *buffer;
    size_t size;
} File;

typedef struct {
    float x;
    float y;
} Vec2f;

typedef struct {
    float x;
    float y;
    float z;
} Vec3f;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4f;

typedef struct {
    float x;
    float y;
    float u;
    float v;
    uint32_t color;
} Vertex;

File read_entire_file(const char *path);
size_t file_size(const char *path);
Vec4f hex_to_vec4f(uint32_t color);

// TODO(marla): make sure we can malloc data and read into that AND
// read into a generic buffer such as the scratch_buffer.
//int read_entire_file_into_buffer(unit8_t *buffer, const char *path);

#endif // COMMON_H_
