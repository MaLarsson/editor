cl src/main.c src/common.c src/editor.c src/opengl.c src/renderer.c src/win32.c /W4 /Iinclude /link freetype.lib msvcrt.lib user32.lib gdi32.lib opengl32.lib /libpath:lib /nodefaultlib:libcmt
