#include "shader.h"
#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

GLboolean compile_shader(GLuint shader_id, const char* src){
    glShaderSource(shader_id, 1, &src , NULL);

    glCompileShader(shader_id);

    GLint isCompiled = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE){
        GLint maxLength = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        GLchar errorLog[maxLength];
        glGetShaderInfoLog(shader_id, maxLength, &maxLength, errorLog);

        printf("%s:%d  Error compiling shader\n %s\n", __FILE__, __LINE__,errorLog);
        // Exit with failure.
        glDeleteShader(shader_id); // Don't leak the shader.
        return GL_FALSE;
    }
    return GL_TRUE;
}

GLuint load_shaders_string(const char* vertex_source,const char* fragment_source){

    // Create the shaders
    GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
    if(!compile_shader(vertex_id,vertex_source)){return 0;}


    GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
    if(!compile_shader(fragment_id,fragment_source)){return 0;}



    // Link the program
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vertex_id);
    glAttachShader(program_id, fragment_id);
    glLinkProgram(program_id);

    GLint isLinked = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE){
        GLint maxLength = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        GLchar infoLog[maxLength];
        glGetProgramInfoLog(program_id, maxLength, &maxLength, &infoLog[0]);

        // We don't need the program anymore.
        glDeleteProgram(program_id);
        // Don't leak shaders either.
        glDeleteShader(vertex_id);
        glDeleteShader(fragment_id);

        printf("%s:%d\nError linking shader\n %s\n", __FILE__, __LINE__, infoLog);

        return 0;
    }

    glDetachShader(program_id, vertex_id);
    glDetachShader(program_id, vertex_id);

    glDeleteShader(vertex_id);
    glDeleteShader(vertex_id);

    return program_id;
}

char* read_file(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Failed to open file %s\n", file_path);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* content = (char*)malloc((file_size + 1) * sizeof(char));
    if (content == NULL) {
        printf("Failed to allocate memory for file %s\n", file_path);
        fclose(file);
        return NULL;
    }

    fread(content, sizeof(char), file_size, file);
    content[file_size] = '\0'; // Null-terminate the string

    fclose(file);
    return content;
}
GLuint load_shaders_file(const char* vertex_file_path, const char* fragment_file_path) {


    // Read the vertex and fragment shader files
    char* vertex_source = read_file(vertex_file_path);
    if (vertex_source == NULL) return 0;

    char* fragment_source = read_file(fragment_file_path);
    if (fragment_source == NULL) {
        free(vertex_source);
        return 0;
    }

    // Use load_shaders_string to compile and link the shaders
    GLuint program_id = load_shaders_string(vertex_source, fragment_source);

    // Clean up
    free(vertex_source);
    free(fragment_source);

    return program_id;
}

void shader_use(shader id){
    glUseProgram(id);
}
