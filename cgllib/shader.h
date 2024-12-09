#pragma once

#include <GL/glew.h>

typedef GLuint shader;

void shader_use(shader);

GLuint load_shaders_file(const char* vertex_file_path, const char* fragment_file_path);

// Macro to update the uniform data
#define shader_set_uniform(program, name, data) _Generic((&data), \
    int*: glUniform1i(glGetUniformLocation(program, name), *(int*)&data), \
    float*: glUniform1f(glGetUniformLocation(program, name), *(float*)&data), \
    float(*)[2]: glUniform2fv(glGetUniformLocation(program, name), 1, (const GLfloat*)data), \
    float(*)[3]: glUniform3fv(glGetUniformLocation(program, name), 1, (const GLfloat*)data), \
    float(*)[4]: glUniform4fv(glGetUniformLocation(program, name), 1, (const GLfloat*)data), \
    float(*)[2][2]: glUniformMatrix2fv(glGetUniformLocation(program, name), 1, GL_FALSE, (const GLfloat*)data), \
    float(*)[3][3]: glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, (const GLfloat*)data), \
    float(*)[4][4]: glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, (const GLfloat*)data) \
)
