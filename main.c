#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#include "cgllib/platform.h"

#include "cgllib/shader.h"
#include "cgllib/mesh.h"
#include "cgllib/camera.h"
#include "cgllib/model.h"

#include "chunk.h"

#include "hashmap.h"
#include "chunkmanager.h"


int sdl_interval_mode=1;
bool running=true;

SDL_Event windowEvent;
bool captureinput=false;
int render_distance=32;

void handle_input(camera *c, platform* p){

    nk_input_begin(p->ctx);
    while(SDL_PollEvent(&windowEvent)){
        if (windowEvent.type == SDL_QUIT) {
            running = false;
        }
        if (windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_TAB){
            captureinput=!captureinput;
            SDL_SetRelativeMouseMode(captureinput);
        }
        if(captureinput){
            if(windowEvent.type == SDL_MOUSEMOTION){
                camera_process_mouse(c,(float)windowEvent.motion.xrel,(float)-windowEvent.motion.yrel,1);
            }
            if(windowEvent.type == SDL_MOUSEWHEEL){
                camera_process_scroll(c, (float)windowEvent.wheel.y);
            }
        }
        nk_sdl_handle_event(&windowEvent);
    }
    //nk_sdl_handle_grab(); /* optional grabbing behavior */
    nk_input_end(p->ctx);
    SDL_PumpEvents();
    const Uint8 *keysArray =  SDL_GetKeyboardState(NULL);
    if(captureinput){
        SDL_WarpMouseInWindow(p->window, p->width / 2, p->height / 2);
        struct {
            SDL_Scancode key1;
            SDL_Scancode key2;
            enum Camera_Motion motion;
        } keyMappings[] = {
                { SDL_SCANCODE_UP, SDL_SCANCODE_W, FORWARD },
                { SDL_SCANCODE_DOWN, SDL_SCANCODE_S, BACKWARD },
                { SDL_SCANCODE_LEFT, SDL_SCANCODE_A, LEFT },
                { SDL_SCANCODE_RIGHT, SDL_SCANCODE_D, RIGHT },
                { SDL_SCANCODE_SPACE, SDL_SCANCODE_SPACE, UP },
                { SDL_SCANCODE_LSHIFT, SDL_SCANCODE_LSHIFT, DOWN }
        };

        for (int i = 0; i < sizeof(keyMappings) / sizeof(keyMappings[0]); ++i) {
            if (keysArray[keyMappings[i].key1] || keysArray[keyMappings[i].key2]) {
                camera_process_input(c, keyMappings[i].motion, p->deltatime);
            }
        }
    }
}

camera c;
//int chunk_distsq(void* c1,void* c2){
//    chunk* cnk1=*(chunk**)c1,*cnk2=*(chunk**)c2;
//    return ((cnk1->x-c.Position[0])*(cnk1->x-c.Position[0]) + (cnk1->y-c.Position[2])*(cnk1->y-c.Position[2]))-
//            (cnk2->x-c.Position[0])*(cnk2->x-c.Position[0]) + (cnk2->y-c.Position[2])*(cnk2->y-c.Position[2]);
//}

uint64_t queue[10000];
int queue_len=0;
mutex_t chunk_queue_lock;

int chunk_thread_count=8;
#define MAX_THREADS 16
uint64_t chunk_staging[MAX_THREADS]={0};

void* chunk_thread(void* id) {
    while (running) {
        mutex_lock(&chunk_queue_lock);
        while (queue_len > 0) {
            chunk_staging[(uint64_t)id] = queue[0];
            memmove(queue, queue + 1, (queue_len-1) * sizeof(uint64_t));
            queue_len--;

            mutex_unlock(&chunk_queue_lock);

            chunk* cnk= get_chunk(SEPPAIR(chunk_staging[(uint64_t)id]));
            generate_chunk_mesh(cnk);
            mutex_lock(&chunk_queue_lock);
        }
        mutex_unlock(&chunk_queue_lock);
        usleep(1000);
    }
    return NULL;
}

int main(void) {
    chunkmap=hashmap_init();
    mutex_init(&chunk_queue_lock);

    platform* p = platform_init(1920, 1080);
    if(!p){exit(1);}

    GLuint framebuffer, texture, renderbuffer;

    // Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a texture to render the scene
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p->width, p->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Attach the texture to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Create a renderbuffer object for depth and stencil (if needed)
    glGenRenderbuffers(1, &renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p->width, p->height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);

    // Check if the framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("err\n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


//    camera c;
//    camera_init(&c,(vec3){8,100,8},(vec3){0,1,0},(vec3){0,0,-1});
    camera_init(&c,(vec3){8,75,8},(vec3){0,1,0},(vec3){0,0,-1});

//    shader flat_shader_3d= load_shaders_file("../shaders/texture3d.vert","../shaders/texture.frag");
//    shader flat_shader_3d= load_shaders_file("../shaders/lighting.vert","../shaders/lighting.frag");
    shader flat_shader_3d= load_shaders_file("../shaders/texlight.vert","../shaders/texlight.frag");
    shader fxaa_shader= load_shaders_file("../shaders/fxaa.vert","../shaders/fxaa.frag");

    if(!flat_shader_3d){
        exit(1);
    }

    //shader_use(flat_shader);
    shader_use(flat_shader_3d);

    pthread_t thread_id[MAX_THREADS];
    for (uint64_t i = 0; i < chunk_thread_count; ++i) {
        pthread_create(&thread_id[i], NULL, chunk_thread, (void*)i);
    }

    while (running){
        platform_clear(p);
        handle_input(&c,p);


        // Reset OpenGL state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
//        glEnable(GL_MULTISAMPLE);
//        glEnable( GL_BLEND );


        // Draw the triangle
        shader_use(flat_shader_3d);

        mat4 projection;
        glm_perspective(glm_rad(c.Zoom), (float)p->width / (float)p->height, .1f, 1000.0f, projection);


        mat4 view;
        camera_get_view_matrix(&c,view);

        mat4 base_mvp;
        glm_mat4_mul(projection,view,base_mvp);
        mat4 model;
        glm_mat4_identity(model);

        shader_set_uniform(flat_shader_3d, "MVP", base_mvp);
        shader_set_uniform(flat_shader_3d, "model", model);
        shader_set_uniform(flat_shader_3d, "view", view);
        shader_set_uniform(flat_shader_3d, "projection", projection);

        vec3 lightpos={0,10000,0};
        vec3 lightdir={.2f,-1.f,.2f};
        glm_normalize(lightdir);
        vec3 color={1,1,1};
        shader_set_uniform(flat_shader_3d, "lightPos", lightpos);
        shader_set_uniform(flat_shader_3d, "lightDir", lightdir);
        shader_set_uniform(flat_shader_3d, "viewPos", c.Position);
        shader_set_uniform(flat_shader_3d, "lightColor", color);
        shader_set_uniform(flat_shader_3d, "objectColor", color);




        int x=((int)c.Position[0])&~0xf;
        int z=((int)c.Position[2])&~0xf;

        chunk* render_list[(render_distance * 2 + 1) * (render_distance * 2 + 1)];
        int r_index=0;

        mutex_lock(&chunk_queue_lock);
//        if(queue_len)
//            printf("removing %d from the queue\n",queue_len);
        queue_len=0;
        mutex_unlock(&chunk_queue_lock);

        for (int dist = 0; dist <= render_distance; ++dist) {
            int startx=x-16*dist,endx=x+16*dist;
            int startz=z-16*dist,endz=z+16*dist;
            for (int w_x = startx; w_x <= endx; w_x+=16) {
                for (int w_z = z-16*dist; w_z <= endz; w_z+=16) {
                    if(w_z==startz+16&&w_x>startx&&w_x<endx){
                        //skip to just before the last in the row
                        w_z=endz;
                    }
                    chunk* cnk = hashmap_lookup(chunkmap, INTPAIR(w_x, w_z));
                    if (!cnk || !cnk->chunkmesh.vertices) {
                        if (queue_len < 400) {
                            mutex_lock(&chunk_queue_lock);
                            int found=0;
                            for (int i = 0; i < chunk_thread_count; ++i) {
                                if(chunk_staging[i]==INTPAIR(w_x, w_z)){
                                    found=1;
                                    break;
                                }
                            }
                            if(!found)
                                queue[queue_len++] = INTPAIR(w_x, w_z);
                            mutex_unlock(&chunk_queue_lock);
                        }
                        continue;
                    }
                    render_list[r_index++] = cnk;
                }
            }
        }
        //minor speedup
        //qsort(render_list,r_index,sizeof(chunk*),(__compar_fn_t)chunk_distsq);

        // glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (int i = 0; i < r_index; ++i) {
            chunk* cnk=render_list[i];
            mesh_draw(&cnk->chunkmesh);
        }

        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //
        // shader_use(fxaa_shader);
        // int t=0;
        // shader_set_uniform(fxaa_shader, "screenTexture", t);
        // vec2 invs={(1.0f/(float)p->width),(1.0f/(float)p->height)};
        // shader_set_uniform(fxaa_shader,"inverseScreenSize",invs);
        //
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture);
        //
        // mesh_draw(&screen_mesh);

        if (nk_begin(p->ctx, "Demo", nk_rect(50, 50, 500, 500),
                     NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
                     NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)){

            nk_layout_row_dynamic(p->ctx, 30, 1);
            nk_label(p->ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(p->ctx, 35, 1);
            if (nk_combo_begin_color(p->ctx, nk_rgb_cf(p->bg), nk_vec2(nk_widget_width(p->ctx),400))) {
                nk_layout_row_dynamic(p->ctx, 150, 1);
                p->bg = nk_color_picker(p->ctx, p->bg, NK_RGBA);
                nk_layout_row_dynamic(p->ctx, 35, 1);
                nk_property_float(p->ctx, "#R:", 0, &p->bg.r, 1.0f, 0.01f,0.005f);
                nk_property_float(p->ctx, "#G:", 0, &p->bg.g, 1.0f, 0.01f,0.005f);
                nk_property_float(p->ctx, "#B:", 0, &p->bg.b, 1.0f, 0.01f,0.005f);
                nk_property_float(p->ctx, "#A:", 0, &p->bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(p->ctx);
            }
            nk_property_int(p->ctx,"Swap interval",-1,&sdl_interval_mode,3,1,0);

            SDL_GL_SetSwapInterval(sdl_interval_mode);
            display_vec3(p->ctx,c.Front,"Front");
            display_vec3(p->ctx,c.Right,"Right");
            display_vec3(p->ctx,c.Up,"Up");
            display_vec3(p->ctx,c.Position,"Pos");
            char fps[64];
            nk_layout_row_dynamic(p->ctx, 35, 1);
            sprintf(fps,"FPS:%.1f ChunkQueue:%d",p->fps,queue_len);
            nk_label(p->ctx, fps, NK_TEXT_LEFT);
            nk_label(p->ctx, "Render Distance", NK_TEXT_LEFT);
            nk_slider_int(p->ctx, 0, &render_distance, 64, 1);

        }
        nk_end(p->ctx);
        nk_sdl_render(NK_ANTI_ALIASING_OFF, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

        platform_swap(p);
    }
    for (int i = 0; i < chunk_thread_count; ++i) {
        pthread_join(thread_id[i],NULL);
    }
    free(chunkmap);
    return 0;
}



