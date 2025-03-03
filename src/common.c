#include "common.h"

File read_entire_file(const char *path) {
    File file;
    file.buffer = NULL;
    file.size = 0;

    FILE *handle = fopen(path, "rb");
    if (!handle) {
        printf("ERROR: Could not open file \"%s\".\n", path);
        return file;
    }

    fseek(handle, 0L, SEEK_END);
    size_t file_size = ftell(handle);
    rewind(handle);

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

