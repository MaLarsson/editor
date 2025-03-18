#ifndef WIN32_H_
#define WIN32_H_

#include "common.h"
#include "editor.h"

typedef struct {
    void *handle;
    void *dc;
    void *opengl_rc;
    bool should_close;
    int width;
    int height;
} Window;

void win32_init_window(Window *window, int width, int height, const char *title);
void win32_poll_events(Window *window, Editor *editor);
void win32_swap_buffers(Window *window);
void win32_swap_interval(int interval);

#endif WIN32_H_
