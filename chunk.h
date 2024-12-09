#pragma once
#include "mutex.h"
#include <cglm/vec3.h>
#include <cglm/vec2.h>
#include "cgllib/mesh.h"


typedef struct{
    mutex_t lock;
    u_int8_t blocks[16][16][256];
    int x,y;
    mesh chunkmesh;
}chunk;

chunk* init_chunk(int x, int y);
void generate_chunk_mesh(chunk* c);


enum blockid{
        air=0,
        grass_block=1,
        dirt_block=2,
        stone_block=3,
        oak_bark=4,
        oak_leaves=5,
        sand_block=6,
        water=7,
        cactus=8,
};
typedef struct{
    int top_texture;
    int side_texture;
    int bottom_texture;
    bool opaque;
}block_type;

static block_type block_types[]={
        [air]={0,0,0,false},
        [grass_block]={0,1,2,true},
        [dirt_block]={2,2,2,true},
        [stone_block]={3,3,3,true},
        [oak_bark]={5,4,5,true},
        [oak_leaves]={6,6,6,true},
        [sand_block]={7,7,7,true},
        [water]={8,8,8,true},
        [cactus]={25,9,25,true},
};
static int NUMBLOCKS= sizeof(block_types)/sizeof(*block_types);