#pragma once
#include "mesh.h"
typedef struct{
    mesh* meshes;
    int mesh_count;
    char* root_dir;
}model;

model load_Model(const char* path);

void model_draw(model* m);