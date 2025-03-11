#include "renderer.h"

static const char *vertex_shader =
    "#version 330 core\n"
    "layout (location = 0) in vec2 pos;\n"
    "layout (location = 1) in vec2 uv;\n"
    "layout (location = 2) in vec4 color;\n"
    "\n"
    "uniform mat4 projection;\n"
    "\n"
    "out vec4 out_color;\n"
    "out vec2 out_uv;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(pos, 0.0, 1.0);\n"
    "    out_uv = uv;\n"
    "    out_color = vec4(color.a, color.b, color.g, color.r);\n"
    "}\n";

static const char *basic_fragment_shader =
    "#version 330 core\n"
    "in vec4 out_color;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = out_color;\n"
    "}\n";

static const char *text_fragment_shader =
    "#version 330 core\n"
    "in vec4 out_color;\n"
    "in vec2 out_uv;\n"
    "\n"
    "uniform sampler2D atlas;\n"
    "\n"
    "void main() {\n"
    "    gl_FragColor = vec4(out_color.rgb, texture(atlas, out_uv).r);\n"
    "}\n";

static compile_shaders(Renderer *renderer) {
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_shader, NULL);
    glCompileShader(vertex);

    GLuint basic_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(basic_shader, 1, &basic_fragment_shader, NULL);
    glCompileShader(basic_shader);

    GLuint text_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(text_shader, 1, &text_fragment_shader, NULL);
    glCompileShader(text_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    // TODO(marla): create both shaders programs.
    glAttachShader(program, basic_shader);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(basic_shader);
    glDeleteShader(text_shader);

    glUseProgram(program);

    renderer->text_program = program;
    renderer->projection_location = glGetUniformLocation(program, "projection");
}

void renderer_init(Renderer *renderer) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    compile_shaders(renderer);

    glGenVertexArrays(1, &renderer->vao);
    glGenBuffers(1, &renderer->vbo);
    glBindVertexArray(renderer->vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)8);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)16);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}

void renderer_reset_draw_data(Renderer *renderer) {
    renderer->vertices.count = 0;
    renderer->indices.count = 0;
    renderer->draw_calls.count = 0;
}

void renderer_update_screen_size(Renderer *renderer, int width, int height) {
    glViewport(0, 0, width, height);

    float w = 2.0f / width;
    float h = 2.0f / height;

    float projection[] = {
         w,  0,  0,  0,
         0,  h,  0,  0,
         0,  0,  1,  0,
        -1, -1,  0,  1,
    };

    glUniformMatrix4fv(renderer->projection_location, 1, GL_FALSE, projection);
}

void renderer_clear_screen(uint32_t color) {
    Vec4f background = hex_to_vec4f(color);
    glClearColor(background.x, background.y, background.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_draw(Renderer *renderer) {
    size_t buffer_size = renderer->vertices.count * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, renderer->vertices.data, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, renderer->vertices.count);
}

void renderer_begin_draw_call(Renderer *renderer, GLuint shader) {
    // ...
}

void renderer_end_draw_call(Renderer *renderer) {
    // ...
}

void render_quad(Renderer *renderer, float x, float y, float w, float h, uint32_t color) {
    Vertex v0 = {x,     y,     0, 1, color};
    Vertex v1 = {x,     y + h, 0, 0, color};
    Vertex v2 = {x + w, y,     1, 1, color};
    Vertex v3 = {x + w, y,     1, 1, color}; // duplicate v2
    Vertex v4 = {x,     y + h, 0, 0, color}; // duplicate v1
    Vertex v5 = {x + w, y + h, 1, 0, color};

    array_add(&renderer->vertices, v0);
    array_add(&renderer->vertices, v1);
    array_add(&renderer->vertices, v2);
    array_add(&renderer->vertices, v3);
    array_add(&renderer->vertices, v4);
    array_add(&renderer->vertices, v5);
}
