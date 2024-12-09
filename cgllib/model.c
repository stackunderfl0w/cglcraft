#include "model.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

model load_Model(const char* path){

    const C_STRUCT aiScene* scene = aiImportFile(path,aiProcess_FlipUVs);
    model m={.meshes=calloc(sizeof(*m.meshes),scene->mNumMeshes),.mesh_count=scene->mNumMeshes, .root_dir=strndup(path,strrchr(path,'/')-path+1)};


    for (int i = 0; i < scene->mNumMeshes; ++i) {
        convert_mesh(&m.meshes[i],scene->mMeshes[i],scene,m.root_dir);
    }
    return m;
}

void model_draw(model* m){
    for (int i = 0; i < m->mesh_count; ++i) {
        mesh_draw(&m->meshes[i]);
    }
}