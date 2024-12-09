
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_GL3_IMPLEMENTATION

#include <sys/time.h>
#include <time.h>
#include "platform.h"

platform* platform_init(int width, int height){
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return NULL;
    }
    platform* p= calloc(sizeof(platform ),1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    // SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    p->window=SDL_CreateWindow("Stackunderfl0w opengl", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if (!p->window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }
    p->context = SDL_GL_CreateContext(p->window);

    GLenum err=glewInit();
    if(err!=GLEW_OK) {
        SDL_Log("glewInit failed: %s\n", glewGetErrorString(err));
        return NULL;
    }

    const GLubyte* version = glGetString(GL_VERSION);
    printf("OpenGL Version: %s\n", version);

    p->ctx=nk_sdl_init(p->window);

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    struct nk_font* font = nk_font_atlas_add_from_file(atlas, "../JupiteroidRegular.ttf", 32, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(p->ctx, &font->handle);
    p->bg=(struct nk_colorf){.r = 0.10f,.g = 0.18f,.b = 0.24f,.a = 1.0f};

    SDL_GetWindowSize(p->window, &p->width, &p->height);

    return p;
}

#define NUM_TIMESTAMPS 60
clock_t timestamps[NUM_TIMESTAMPS];
int time_idx=0;
#define seconds ((int)1e6)

//replace clock() which counts program execution time and freezes during sleep/usleep
clock_t clock_mono(){
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return current_time.tv_sec*(int)1e6+current_time.tv_usec;
}
clock_t last_update=0;

void platform_clear(platform* p) {
    SDL_GetWindowSize(p->window, &p->width, &p->height);
    glViewport(0, 0, p->width, p->height);
    glClearColor(p->bg.r,p->bg.g,p->bg.b,p->bg.a);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    timestamps[time_idx%NUM_TIMESTAMPS]=clock_mono();
    double dtime=(timestamps[time_idx%NUM_TIMESTAMPS]-timestamps[(time_idx+1)%NUM_TIMESTAMPS])/(double)seconds;
    double fps=(NUM_TIMESTAMPS-1)/dtime;
    if(timestamps[time_idx%NUM_TIMESTAMPS]>(last_update+seconds)){
        printf("fps:%.2f\n",fps);
        last_update=timestamps[time_idx%NUM_TIMESTAMPS];
    }
    p->fps=(float)fps;
    p->deltatime=((float)(unsigned int)(timestamps[time_idx%NUM_TIMESTAMPS]-timestamps[(time_idx+NUM_TIMESTAMPS-1)%NUM_TIMESTAMPS]))/(float)seconds;
    time_idx=(time_idx+1)%NUM_TIMESTAMPS;

}


void platform_swap(platform* p){
    SDL_GL_SwapWindow(p->window);
}

void display_vec3(struct nk_context *ctx, float* v, const char *label) {
    nk_layout_row_dynamic(ctx, 30, 4); // 3 components, one for each float (x, y, z)

    nk_label(ctx, label, NK_TEXT_LEFT);
    nk_property_float(ctx, "#X:", -FLT_MAX, &v[0], FLT_MAX, 0.1f, 0.1f);
    nk_property_float(ctx, "#Y:", -FLT_MAX, &v[1], FLT_MAX, 0.1f, 0.1f);
    nk_property_float(ctx, "#Z:", -FLT_MAX, &v[2], FLT_MAX, 0.1f, 0.1f);
}




