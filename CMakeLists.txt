cmake_minimum_required(VERSION 3.30)
project(cglcraft C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Ofast -mtune=native")

add_executable(cglcraft main.c cgllib/platform.c cgllib/shader.c cgllib/mesh.c cgllib/camera.c cgllib/model.c cgllib/texture.c
        chunkmanager.c
        chunk.c
        mutex.c
        hashmap.c)

target_link_libraries(cglcraft GLEW OpenGL SDL2 cglm assimp m)
