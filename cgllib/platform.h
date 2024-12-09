#pragma once

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include <nuklear.h>
#include "nuklear_sdl_gl3.h"
#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024


struct platform_sdl2_gl{
    SDL_Window* window;
    SDL_GLContext context;
    struct nk_context *ctx;
    struct nk_colorf bg;
    int width,height;
    float deltatime;
    float fps;
};

typedef struct platform_sdl2_gl platform;

platform* platform_init(int width, int height);

void platform_clear(platform* p);

void platform_swap(platform* p);

void display_vec3(struct nk_context *ctx, float* v, const char *label);
