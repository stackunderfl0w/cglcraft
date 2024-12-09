#pragma once
#include <GL/glew.h>
#include <cglm/cglm.h>
#include <assimp/scene.h>
#include "texture.h"
typedef struct{
    vec3* vertices;
    unsigned vertex_count;
    vec2* texuvs;
    vec3* normals;
    vec3* indexes;
    unsigned index_count;
    texture* textures;
    unsigned texture_count;
    GLuint VAO;
    GLboolean indexed;
    GLuint type;
}mesh;

extern mesh screen_mesh;

void convert_mesh(mesh* m, struct aiMesh *mesh, const struct aiScene *scene, const char* dir);

GLuint mesh_buffer(mesh* m);

void mesh_draw(mesh* m);
