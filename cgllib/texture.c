#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_TEXTURES 100

typedef struct {
    char name[256];
    GLuint ID;
} TextureCache;

TextureCache textureCache[MAX_TEXTURES];
int cacheCount = 0;

GLuint get_texture_from_cache(const char* name) {
    for (int i = 0; i < cacheCount; i++) {
        if (strcmp(textureCache[i].name, name) == 0) {
            return textureCache[i].ID;
        }
    }
    return 0; // Not found
}


void texture_load_from_disk(texture* t){
    t->data = stbi_load(t->name, &t->width, &t->height, &t->nrComponents, 0);
}

GLuint texture_buffer(texture* t){
    if(t->ID)//already loaded
        return t->ID;

    t->ID = get_texture_from_cache(t->name);
    if (t->ID){
        return t->ID;
    }

    if(!t->data){//not loaded from disk
        texture_load_from_disk(t);
        if(!t->data)
            return 0;
    }
    GLint format=t->nrComponents == 4?GL_RGBA:t->nrComponents == 3?GL_RGB:t->nrComponents == 1?GL_RED:0;
    if(!format)
        return 0;

    glGenTextures(1, &t->ID);
    glBindTexture(GL_TEXTURE_2D, t->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, t->width, t->height, 0, format, GL_UNSIGNED_BYTE, t->data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(t->data);

    if (cacheCount < MAX_TEXTURES) {
        strncpy(textureCache[cacheCount].name, t->name, sizeof(textureCache[cacheCount].name) - 1);
        textureCache[cacheCount].ID = t->ID;
        cacheCount++;
    }

    return t->ID;
}

void texture_bind(texture* t, GLuint index){
    if(!t->ID)
        texture_buffer(t);
    glActiveTexture(GL_TEXTURE0 + index);

    glBindTexture(GL_TEXTURE_2D, t->ID);
}