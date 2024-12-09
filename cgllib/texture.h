#pragma once
#include <GL/glew.h>
#include <assimp/material.h>
typedef struct {
    GLuint ID;
    char* name;
    enum aiTextureType type;

    unsigned char* data;
    int width, height, nrComponents;

}texture;

void texture_load_from_disk(texture* t);

GLuint texture_buffer(texture* t);

void texture_bind(texture* t, GLuint index);