#include "win32.h"
#include "opengl.h"

#include <windows.h>

#define WGL_DRAW_TO_WINDOW_ARB 0x2001
#define WGL_ACCELERATION_ARB   0x2003
#define WGL_SUPPORT_OPENGL_ARB 0x2010
#define WGL_DOUBLE_BUFFER_ARB  0x2011
#define WGL_PIXEL_TYPE_ARB     0x2013
#define WGL_COLOR_BITS_ARB     0x2014
#define WGL_DEPTH_BITS_ARB     0x2022
#define WGL_STENCIL_BITS_ARB   0x2023

#define WGL_FULL_ACCELERATION_ARB 0x2027
#define WGL_TYPE_RGBA_ARB         0x202B

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20A9

#define WGL_CONTEXT_MAJOR_VERSION_ARB    0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB    0x2092
#define WGL_CONTEXT_FLAGS_ARB            0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB     0x9126

typedef BOOL (WINAPI *WGLSwapIntervalExtProc)(int interval);
typedef HGLRC (WINAPI *WGLCreateContextAttribsARBProc)(HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL (WINAPI *WGLChoosePixelFormatARBProc)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);

static bool ctrl_down = false;

static WGLSwapIntervalExtProc wglSwapIntervalEXT;
static WGLCreateContextAttribsARBProc wglCreateContextAttribsARB;
static WGLChoosePixelFormatARBProc wglChoosePixelFormatARB;

static int window_width = 1200;
static int window_height = 1200;

static Window *g_window = NULL;

LRESULT CALLBACK main_window_callback(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    LRESULT result = 0;
    switch (message) {
    case WM_MOUSEWHEEL:
        g_window->scroll -= GET_WHEEL_DELTA_WPARAM(w_param);
        break;
    case WM_KEYDOWN:
        if (w_param == VK_RIGHT) g_window->cursor += 1;
        if (w_param == VK_LEFT) g_window->cursor -= 1;
        if (w_param == VK_CONTROL) ctrl_down = true;
        if (w_param == 'F' && ctrl_down) g_window->cursor += 1;
        if (w_param == 'B' && ctrl_down) g_window->cursor -= 1;
        break;
    case WM_KEYUP:
        if (w_param == VK_CONTROL) ctrl_down = false;
        break;
    case WM_SIZE:
        g_window->width = LOWORD(l_param);
        g_window->height = HIWORD(l_param);
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        g_window->should_close = true;
        break;
    default:
        result = DefWindowProcA(window, message, w_param, l_param);
        break;
    }
    return result;
}

static void win32_init_opengl_extensions(void) {
    WNDCLASSA dummy_window_class = {0};
    dummy_window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    dummy_window_class.lpfnWndProc = DefWindowProcA;
    dummy_window_class.hInstance = GetModuleHandle(NULL);
    dummy_window_class.lpszClassName = "wgl_loader";

    if (!RegisterClassA(&dummy_window_class)) {
        //fatal_error("Failed to register dummy OpenGL window.");
    }

    HWND dummy_window = CreateWindowExA(0, dummy_window_class.lpszClassName, "", 0,
                                        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                        0, 0, dummy_window_class.hInstance, 0);
    if (!dummy_window) {
        //fatal_error("Failed to create dummy OpenGL window.");
    }

    HDC dummy_dc = GetDC(dummy_window);

    PIXELFORMATDESCRIPTOR desired_pixel_format = {0};
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.cStencilBits = 8;

    int pixel_format_index = ChoosePixelFormat(dummy_dc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR pixel_format;
    DescribePixelFormat(dummy_dc, pixel_format_index, sizeof(pixel_format), &pixel_format);
    SetPixelFormat(dummy_dc, pixel_format_index, &pixel_format);

    HGLRC dummy_rc = wglCreateContext(dummy_dc);
    if (!wglMakeCurrent(dummy_dc, dummy_rc)) {
        //printf("unable to create opengl context\n");
        //window_should_close = true;
    }

    wglSwapIntervalEXT = (WGLSwapIntervalExtProc)wglGetProcAddress("wglSwapIntervalEXT");
    wglCreateContextAttribsARB = (WGLCreateContextAttribsARBProc)wglGetProcAddress("wglCreateContextAttribsARB");
    wglChoosePixelFormatARB = (WGLChoosePixelFormatARBProc)wglGetProcAddress("wglChoosePixelFormatARB");

    init_opengl((GetProcAddressProc)wglGetProcAddress);

    if (!wglSwapIntervalEXT || !wglCreateContextAttribsARB || !wglChoosePixelFormatARB) {
        //printf("unable to load required opengl extensions\n");
        //window_should_close = true;
    }

    wglMakeCurrent(dummy_dc, 0);
    wglDeleteContext(dummy_rc);
    ReleaseDC(dummy_window, dummy_dc);
    DestroyWindow(dummy_window);
    UnregisterClassA(dummy_window_class.lpszClassName, dummy_window_class.hInstance);
}

static HGLRC win32_init_opengl(HDC window_dc) {
    win32_init_opengl_extensions();

    int pixel_format_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB,           GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB,           GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,            GL_TRUE,
        WGL_ACCELERATION_ARB,             WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,               WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,               32,
        WGL_DEPTH_BITS_ARB,               24,
        WGL_STENCIL_BITS_ARB,             8,
        WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
        0,
    };

    int pixel_format_index;
    UINT num_formats;
    wglChoosePixelFormatARB(window_dc, pixel_format_attribs, 0, 1, &pixel_format_index, &num_formats);
    if (!num_formats) {
        // ...
    }

    PIXELFORMATDESCRIPTOR pixel_format;
    DescribePixelFormat(window_dc, pixel_format_index, sizeof(pixel_format), &pixel_format);
    SetPixelFormat(window_dc, pixel_format_index, &pixel_format);

    int opengl_context_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    HGLRC opengl_rc = wglCreateContextAttribsARB(window_dc, 0, opengl_context_attribs);

    if (!wglMakeCurrent(window_dc, opengl_rc)) {
        printf("unable to create opengl context\n");
        //window_should_close = true;
    }

    return opengl_rc;
}

void win32_init_window(Window *window, int width, int height, const char *title) {
    // TODO(marla): remove these?
    g_window = window;
    window->scroll = 0;
    window->cursor = 0;

    window->width = 1200;
    window->height = 1200;

    WNDCLASSA window_class = {0};
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    window_class.lpfnWndProc = main_window_callback;
    window_class.hInstance = GetModuleHandle(NULL);
    window_class.lpszClassName = "MainWindow";

    if (!RegisterClassA(&window_class)) {
        printf("unable to register window class\n");
        //return 1;
    }

    window->handle = CreateWindowExA(0, window_class.lpszClassName, "Hello, World!", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, window_class.hInstance, 0);
    if (!window->handle) {
        printf("unable to create window\n");
        //return 2;
    }

    window->dc = GetDC(window->handle);
    window->opengl_rc = win32_init_opengl(window->dc);
}

void win32_poll_events(void) {
    MSG message;
    while (PeekMessage(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }
}

void win32_swap_buffers(Window *window) {
    SwapBuffers(window->dc);
}

void win32_swap_interval(int interval) {
    wglSwapIntervalEXT(interval);
}
