#pragma once
#include "hashmap.h"
#include "chunk.h"

#define INTPAIR(a,b) ((uint64_t)(((uint64_t)(uint32_t)a)<<32)|((uint32_t)b))

#define SEPPAIR(x) ((int)(uint32_t)(x >> 32)), ((int)(uint32_t)x)

extern HashMap* chunkmap;

chunk* get_chunk(int x, int y);
