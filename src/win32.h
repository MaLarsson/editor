#ifndef WIN32_H_
#define WIN32_H_

#include "common.h"

typedef struct {
    void *handle;
    void *dc;
    void *opengl_rc;
    bool should_close;
    int width;
    int height;
} Window;

void win32_create_window(Window *window, int width, int height, const char *title);
void win32_poll_events(void);
void win32_swap_buffers(Window *window);
void win32_swap_interval(int interval);

#endif WIN32_H_
