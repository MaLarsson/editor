#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

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
    float u;
    float v;
    uint32_t color;
} Vertex;

File read_entire_file(const char *path);

#endif // COMMON_H_
