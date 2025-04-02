#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define UNUSED(x) (void)(x)

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

#define KB(sz) (1024*(sz))
#define MB(sz) (1024*1024*(sz))

#define MAX_SCRATCH_BUFFER MB(10)

typedef struct {
    uint8_t data[MAX_SCRATCH_BUFFER];
    uint32_t len;
} Scratch;

typedef struct {
    char *buffer;
    size_t size;
} File;

File read_entire_file(const char *path);
size_t file_size(const char *path);

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

Vec4f hex_to_vec4f(uint32_t color);

typedef struct {
    float x;
    float y;
    float u;
    float v;
    uint32_t color;
} Vertex;

typedef struct {
    const char *data;
    size_t len;
} StringView;

// TODO(marla): make sure we can malloc data and read into that AND
// read into a generic buffer such as the scratch_buffer.
//int read_entire_file_into_buffer(unit8_t *buffer, const char *path);

typedef struct {
    char *data;
    size_t count;
    size_t capacity;
    size_t gap;
} GapBuffer;

size_t gap_buffer_index(GapBuffer *buffer, size_t index);
char gap_buffer_at(GapBuffer *buffer, size_t index);
void gap_buffer_init(GapBuffer *buffer, size_t capacity);
void gap_buffer_insert(GapBuffer *buffer, size_t index, const char *string, size_t len);
void gap_buffer_insert_string(GapBuffer *buffer, size_t index, const char *string);
void gap_buffer_insert_char(GapBuffer *buffer, size_t index, char c);
void gap_buffer_dump(GapBuffer *buffer);

#endif // COMMON_H_
