#include "chunk.h"
#include <stdlib.h>
#include <string.h>
#include "snoise/snoise3.h"
#include "chunkmanager.h"

NoiseContext nc;
int init=0;

enum face{
    FRONT,
    BACK,
    LEFT,
    RIGHT,
    BOTTOM,
    TOP,
};


chunk* init_chunk(int world_x, int world_y) {
    chunk *c = calloc(sizeof(*c), 1);
    mutex_init(&c->lock);
    c->x = world_x;
    c->y = world_y;
    if (!init) {
        permsetup(&nc);
    }

    for (int x = 0; x < 16; ++x) {
        for (int y = 0; y < 16; ++y) {
            // base height
            float base_noise = noise3(0, ((float)(world_x + x)) / 100.0f, ((float)(world_y + y)) / 100.0f, &nc);
            float detail_noise = 5 * noise3(1, ((float)(world_x + x)) / 50.0f, ((float)(world_y + y)) / 50.0f, &nc);
            int dirt_height = 60 + 16 * base_noise + detail_noise;

            // Biomes
            float biome_noise = noise3(2, ((float)(world_x + x)) / 800.0f, ((float)(world_y + y)) / 800.0f, &nc);
            int block_type = dirt_block;

            if (biome_noise < 0.15) {
                // Mountain
                dirt_height += 20;
                block_type = (dirt_height > 70) ? stone_block : dirt_block;
            } else if (biome_noise > 0.55) {
                // Desert
                dirt_height -= 10;
                block_type = sand_block;
            } else {
                // Grassland
                block_type = dirt_block;
            }

            memset(&c->blocks[x][y][0], block_type, dirt_height);
            if(block_type==dirt_block)
                c->blocks[x][y][dirt_height] = grass_block;
        }
    }
    return c;
}


void add_quad(vec3 *quads, int x, int y, int z, enum face face) {
    switch (face) {
        case FRONT: // facing -Z
            glm_vec3_copy((vec3){x, y, z}, quads[3]);
            glm_vec3_copy((vec3){x, y + 1, z}, quads[0]);
            glm_vec3_copy((vec3){x + 1, y + 1, z}, quads[1]);
            glm_vec3_copy((vec3){x + 1, y, z}, quads[2]);
            break;

        case BACK: // facing +Z
            glm_vec3_copy((vec3){x, y, z + 1}, quads[2]);
            glm_vec3_copy((vec3){x + 1, y, z + 1}, quads[3]);
            glm_vec3_copy((vec3){x + 1, y + 1, z + 1}, quads[0]);
            glm_vec3_copy((vec3){x, y + 1, z + 1}, quads[1]);
            break;

        case LEFT: // facing -X
            glm_vec3_copy((vec3){x, y, z}, quads[2]);
            glm_vec3_copy((vec3){x, y, z + 1}, quads[3]);
            glm_vec3_copy((vec3){x, y + 1, z + 1}, quads[0]);
            glm_vec3_copy((vec3){x, y + 1, z}, quads[1]);
            break;

        case RIGHT: // facing +X

            glm_vec3_copy((vec3){x + 1, y, z}, quads[3]);
            glm_vec3_copy((vec3){x + 1, y + 1, z}, quads[0]);
            glm_vec3_copy((vec3){x + 1, y + 1, z + 1}, quads[1]);
            glm_vec3_copy((vec3){x + 1, y, z + 1}, quads[2]);
            break;

        case BOTTOM: // facing -Y
            glm_vec3_copy((vec3){x, y, z}, quads[0]);
            glm_vec3_copy((vec3){x + 1, y, z}, quads[1]);
            glm_vec3_copy((vec3){x + 1, y, z + 1}, quads[2]);
            glm_vec3_copy((vec3){x, y, z + 1}, quads[3]);
            break;

        case TOP: // facing +Y
            glm_vec3_copy((vec3){x, y + 1, z}, quads[0]);
            glm_vec3_copy((vec3){x, y + 1, z + 1}, quads[1]);
            glm_vec3_copy((vec3){x + 1, y + 1, z + 1}, quads[2]);
            glm_vec3_copy((vec3){x + 1, y + 1, z}, quads[3]);
            break;
    }
}

void add_normals(vec3 *normals, enum face face) {
    switch (face) {
        case FRONT: // facing -Z
            glm_vec3_copy((vec3){0, 0, -1}, normals[0]);
            glm_vec3_copy((vec3){0, 0, -1}, normals[1]);
            glm_vec3_copy((vec3){0, 0, -1}, normals[2]);
            glm_vec3_copy((vec3){0, 0, -1}, normals[3]);
            break;

        case BACK: // facing +Z
            glm_vec3_copy((vec3){0, 0, 1}, normals[0]);
            glm_vec3_copy((vec3){0, 0, 1}, normals[1]);
            glm_vec3_copy((vec3){0, 0, 1}, normals[2]);
            glm_vec3_copy((vec3){0, 0, 1}, normals[3]);
            break;

        case LEFT: // facing -X
            glm_vec3_copy((vec3){-1, 0, 0}, normals[0]);
            glm_vec3_copy((vec3){-1, 0, 0}, normals[1]);
            glm_vec3_copy((vec3){-1, 0, 0}, normals[2]);
            glm_vec3_copy((vec3){-1, 0, 0}, normals[3]);
            break;

        case RIGHT: // facing +X
            glm_vec3_copy((vec3){1, 0, 0}, normals[0]);
            glm_vec3_copy((vec3){1, 0, 0}, normals[1]);
            glm_vec3_copy((vec3){1, 0, 0}, normals[2]);
            glm_vec3_copy((vec3){1, 0, 0}, normals[3]);
            break;

        case BOTTOM: // facing -Y
            glm_vec3_copy((vec3){0, -1, 0}, normals[0]);
            glm_vec3_copy((vec3){0, -1, 0}, normals[1]);
            glm_vec3_copy((vec3){0, -1, 0}, normals[2]);
            glm_vec3_copy((vec3){0, -1, 0}, normals[3]);
            break;

        case TOP: // facing +Y
            glm_vec3_copy((vec3){0, 1, 0}, normals[0]);
            glm_vec3_copy((vec3){0, 1, 0}, normals[1]);
            glm_vec3_copy((vec3){0, 1, 0}, normals[2]);
            glm_vec3_copy((vec3){0, 1, 0}, normals[3]);
            break;
    }
}



void add_uv(vec2 *uvs, int tex_index) {
    float uv_x = (float)(tex_index%16)/16.0f;
    float uv_y = (float)(tex_index/16)/16.0f;

    glm_vec2_copy((vec2){uv_x           , uv_y                }, uvs[0]);
    glm_vec2_copy((vec2){uv_x+1.0f/16.0f, uv_y                }, uvs[1]);
    glm_vec2_copy((vec2){uv_x+1.0f/16.0f, (uv_y + 1.0f/16.0f) }, uvs[2]);
    glm_vec2_copy((vec2){uv_x           , (uv_y + 1.0f/16.0f) }, uvs[3]);
}

void generate_chunk_mesh(chunk* c) {
    chunk *n = get_chunk(c->x, c->y + 16);  // north z+
    chunk *s = get_chunk(c->x, c->y - 16);  // south z-
    chunk *e = get_chunk(c->x + 16, c->y);  // east x+
    chunk *w = get_chunk(c->x - 16, c->y);  // west x-
    int count = 0;

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            for (int y = 0; y < 256; ++y) {
                if (c->blocks[x][z][y]) {
                    // Check each face and count if exposed

                    // Front face (negative Z direction)
                    if ((z == 0 && !s->blocks[x][15][y]) || (z != 0 && !c->blocks[x][z - 1][y])) count++;

                    // Back face (positive Z direction)
                    if ((z == 15 && !n->blocks[x][0][y]) || (z != 15 && !c->blocks[x][z + 1][y])) count++;

                    // Left face (negative X direction)
                    if ((x == 0 && !w->blocks[15][z][y]) || (x != 0 && !c->blocks[x - 1][z][y])) count++;

                    // Right face (positive X direction)
                    if ((x == 15 && !e->blocks[0][z][y]) || (x != 15 && !c->blocks[x + 1][z][y])) count++;

                    // Bottom face (negative Y direction)
                    if (y == 0 || !c->blocks[x][z][y - 1]) count++;

                    // Top face (positive Y direction)
                    if (y == 255 || !c->blocks[x][z][y + 1]) count++;
                }
            }
        }
    }

    vec3* quads = malloc(4 * count * sizeof(*quads));
    vec2* uvs = malloc(4 * count * sizeof(*uvs));
    vec3* normals = malloc(4 * count * sizeof(*normals));

    int quad_index = 0;

    for (int x = 0; x < 16; ++x) {
        for (int z = 0; z < 16; ++z) {
            for (int y = 0; y < 256; ++y) {
                if (c->blocks[x][z][y]) {
                    int block_type = c->blocks[x][z][y];

                    // Front face (negative Z direction)
                    if ((z == 0 && !s->blocks[x][15][y]) || (z != 0 && !c->blocks[x][z - 1][y])) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, FRONT);
                        add_uv(&uvs[quad_index], block_types[block_type].side_texture);
                        add_normals(&normals[quad_index],FRONT);
                        quad_index += 4;
                    }

                    // Back face (positive Z direction)
                    if ((z == 15 && !n->blocks[x][0][y]) || (z != 15 && !c->blocks[x][z + 1][y])) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, BACK);
                        add_uv(&uvs[quad_index], block_types[block_type].side_texture);
                        add_normals(&normals[quad_index],BACK);
                        quad_index += 4;
                    }

                    // Left face (negative X direction)
                    if ((x == 0 && !w->blocks[15][z][y]) || (x != 0 && !c->blocks[x - 1][z][y])) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, LEFT);
                        add_uv(&uvs[quad_index], block_types[block_type].side_texture);
                        add_normals(&normals[quad_index],LEFT);
                        quad_index += 4;
                    }

                    // Right face (positive X direction)
                    if ((x == 15 && !e->blocks[0][z][y]) || (x != 15 && !c->blocks[x + 1][z][y])) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, RIGHT);
                        add_uv(&uvs[quad_index], block_types[block_type].side_texture);
                        add_normals(&normals[quad_index],RIGHT);
                        quad_index += 4;
                    }

                    // Bottom face (negative Y direction)
                    if (y == 0 || !c->blocks[x][z][y - 1]) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, BOTTOM);
                        add_uv(&uvs[quad_index], block_types[block_type].bottom_texture);
                        add_normals(&normals[quad_index],BOTTOM);
                        quad_index += 4;
                    }

                    // Top face (positive Y direction)
                    if (y == 255 || !c->blocks[x][z][y + 1]) {
                        add_quad(&quads[quad_index], x+c->x, y, z+c->y, TOP);
                        add_uv(&uvs[quad_index], block_types[block_type].top_texture);
                        add_normals(&normals[quad_index],TOP);
                        quad_index += 4;
                    }
                }
            }
        }
    }

    c->chunkmesh.vertices = quads;
    c->chunkmesh.texuvs = uvs;
    c->chunkmesh.normals = normals;
    c->chunkmesh.vertex_count = quad_index;

    c->chunkmesh.texture_count = 1;
    c->chunkmesh.textures = calloc(sizeof(*c->chunkmesh.textures), 1);
    c->chunkmesh.textures[0].name = strdup("DefaultPack.png");

    c->chunkmesh.type=GL_QUADS;
}
