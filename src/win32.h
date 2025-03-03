#ifndef WIN32_H_
#define WIN32_H_

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

extern WGLSwapIntervalExtProc wglSwapIntervalEXT;
extern WGLCreateContextAttribsARBProc wglCreateContextAttribsARB;
extern WGLChoosePixelFormatARBProc wglChoosePixelFormatARB;

void win32_swap_interval(int interval);

#endif WIN32_H_
