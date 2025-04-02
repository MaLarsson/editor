#include "common.h"

static Scratch scratch_buffer = {0};

size_t file_size(const char *path) {
    FILE *handle = fopen(path, "rb");
    if (!handle) return 0;

    fseek(handle, 0, SEEK_END);
    size_t file_size = ftell(handle);
    fclose(handle);

    return file_size;
}

File read_entire_file(const char *path) {
    File file;
    file.buffer = NULL;
    file.size = 0;

    FILE *handle = fopen(path, "rb");
    if (!handle) {
        printf("ERROR: Could not open file \"%s\".\n", path);
        return file;
    }

    fseek(handle, 0, SEEK_END);
    size_t file_size = ftell(handle);
    fseek(handle, 0, SEEK_SET);

    file.buffer = malloc(sizeof(char) * file_size + 1);
    if (!file.buffer) {
        printf("ERROR: Not enough memory to read \"%s\".\n", path);
        fclose(handle);
        return file;
    }
    file.size = file_size;

    size_t bytes_read = fread(file.buffer, sizeof(char), file_size, handle);
    if (bytes_read < file_size) {
        printf("ERROR: Could not read file \"%s\".\n", path);
        free(file.buffer);
        file.buffer = NULL;
        fclose(handle);
        return file;
    }

    file.buffer[bytes_read] = '\0';
    fclose(handle);

    return file;
}

Vec4f hex_to_vec4f(uint32_t color) {
    Vec4f result;
    result.x = ((color >> (3*8)) & 0xFF) / 255.0f;
    result.y = ((color >> (2*8)) & 0xFF) / 255.0f;
    result.z = ((color >> (1*8)) & 0xFF) / 255.0f;
    result.w = ((color >> (0*8)) & 0xFF) / 255.0f;
    return result;
}

static size_t gap_buffer_gap_size(GapBuffer *buffer) {
    return buffer->capacity - buffer->count;
}

static void gap_buffer_grow(GapBuffer *buffer, size_t size) {
    size_t gap_size = gap_buffer_gap_size(buffer);
    buffer->capacity = size;
    buffer->data = realloc(buffer->data, buffer->capacity);

    size_t len = buffer->count - buffer->gap;
    char *tail_start = &buffer->data[buffer->gap + gap_size];
    char *new_tail_start = &buffer->data[buffer->gap + gap_buffer_gap_size(buffer)];
    memmove(new_tail_start, tail_start, len);
}

static void gap_buffer_move_gap(GapBuffer *buffer, size_t index) {
    size_t gap_size = gap_buffer_gap_size(buffer);

    if (index < buffer->gap) {
        size_t len = buffer->gap - index;
        memmove(&buffer->data[buffer->gap + gap_size - len], &buffer->data[index], len);
    } else {
        size_t len = index - buffer->gap;
        memmove(&buffer->data[buffer->gap], &buffer->data[buffer->gap + gap_size], len);
    }

    buffer->gap = index;
}

size_t gap_buffer_index(GapBuffer *buffer, size_t index) {
    if (index >= buffer->gap) {
        return index + gap_buffer_gap_size(buffer);
    }
    return index;
}

char gap_buffer_at(GapBuffer *buffer, size_t index) {
    return buffer->data[gap_buffer_index(buffer, index)];
}

void gap_buffer_init(GapBuffer *buffer, size_t capacity) {
    buffer->data = malloc(sizeof(char) * capacity);
    buffer->count = 0;
    buffer->capacity = capacity;
    buffer->gap = 0;
}

void gap_buffer_insert(GapBuffer *buffer, size_t index, const char *string, size_t len) {
    if (index != buffer->gap) gap_buffer_move_gap(buffer, index);

    memcpy(&buffer->data[buffer->gap], string, len);
    buffer->gap += len;
    buffer->count += len;
}

void gap_buffer_insert_string(GapBuffer *buffer, size_t index, const char *string) {
    size_t len = strlen(string);
    if (index != buffer->gap) gap_buffer_move_gap(buffer, index);

    memcpy(&buffer->data[buffer->gap], string, len);
    buffer->gap += len;
    buffer->count += len;
}

void gap_buffer_insert_char(GapBuffer *buffer, size_t index, char c) {
    if (index != buffer->gap) gap_buffer_move_gap(buffer, index);

    buffer->data[buffer->gap++] = c;
    buffer->count += 1;
}

void gap_buffer_dump(GapBuffer *buffer) {
    printf("count: %llu, capacity: %llu\n", buffer->count, buffer->capacity);
    printf("data: \"");

    for (size_t i = 0; i < buffer->gap; ++i) {
        printf("%c", buffer->data[i]);
    }

    printf("[");
    size_t gap_size = gap_buffer_gap_size(buffer);
    for (size_t i = 0; i < gap_size; ++i) {
        printf(" ");
    }
    printf("]");

    for (size_t i = buffer->gap + gap_size; i < buffer->capacity; ++i) {
        printf("%c", buffer->data[i]);
    }

    printf("\"\n");
}
