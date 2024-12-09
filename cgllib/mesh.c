#include "mesh.h"

vec3 quadVertices[] = {
    // positions
    {-1.0f,  1.0f,0},  // Top-left
    {-1.0f, -1.0f,0},  // Bottom-left
    {1.0f, -1.0f,0},  // Bottom-right

    {-1.0f,  1.0f,0},  // Top-left
    {1.0f, -1.0f,0},  // Bottom-right
    {1.0f,  1.0f,0}   // Top-right
};

vec2 quadUVs[] = {
    {0.0f, 1.0f}, // Top-left
    {0.0f, 0.0f}, // Bottom-left
    {1.0f, 0.0f}, // Bottom-right

    {0.0f, 1.0f}, // Top-left
    {1.0f, 0.0f}, // Bottom-right
    {1.0f, 1.0f}  // Top-right
};

mesh screen_mesh = {
    .vertices = quadVertices,
    .vertex_count = sizeof(quadVertices) / sizeof(quadVertices[0]),
    .texuvs = quadUVs,
    .type = GL_TRIANGLES,
};

void convert_mesh(mesh* m, struct aiMesh *mesh, const struct aiScene *scene, const char* dir){
    m->vertices=malloc(mesh->mNumVertices*sizeof(*mesh->mVertices));
    memcpy(m->vertices,mesh->mVertices,mesh->mNumVertices*sizeof(*mesh->mVertices));
    m->vertex_count=mesh->mNumVertices;

    m->texuvs=malloc(mesh->mNumVertices*sizeof(*mesh->mTextureCoords));

    for (int i = 0; i < mesh->mNumVertices; ++i) {
        if(mesh->mTextureCoords[0])
            memcpy(&m->texuvs[i],&mesh->mTextureCoords[0][i],sizeof(vec2));
    }


    struct aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    unsigned int num_textures = aiGetMaterialTextureCount(material,aiTextureType_DIFFUSE);
    m->textures= calloc(sizeof(*m->textures),num_textures);
    m->texture_count=num_textures;
    struct aiString str;
    for (int i = 0; i < num_textures; ++i) {
        aiGetMaterialTexture(material,aiTextureType_DIFFUSE,i,&str,NULL,NULL,NULL,NULL,NULL,NULL);

        m->textures[i]=(texture){.name=malloc(strlen(dir)+str.length+1),.type=aiTextureType_DIFFUSE};
        strcpy(m->textures[i].name,dir);
        strncat(m->textures[i].name,str.data,str.length);
    }
}

GLuint mesh_buffer(mesh* m){
    // Generate and bind VAO
    glGenVertexArrays(1, &m->VAO);
    glBindVertexArray(m->VAO);

    // Generate and bind VBO for positions
    GLuint vbo[3];
    glGenBuffers(3, vbo);

    // Positions
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertex_count, m->vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // UVs
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * m->vertex_count, m->texuvs, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // Normals
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertex_count, m->normals, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (GLvoid*)0);
    glEnableVertexAttribArray(2);

    // Unbind the VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return m->VAO; // Return VAO if needed
}
void mesh_draw(mesh* m){
    if(!m->VAO)
        mesh_buffer(m);
    for (int i = 0; i < m->texture_count; ++i) {
        texture_bind(&m->textures[i],i);
    }
    glBindVertexArray(m->VAO);
    glDrawArrays(m->type?m->type:GL_TRIANGLES, 0, (int)m->vertex_count);
    // glDrawArrays(GL_TRIANGLES, 0, (int)m->vertex_count);
    glBindVertexArray(0);
}