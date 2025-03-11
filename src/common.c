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
