#include "chunkmanager.h"

HashMap* chunkmap;

mutex_t chunklock={0};

chunk *get_chunk(int x, int y) {
    mutex_lock(&chunklock);
    chunk* cnk = hashmap_lookup(chunkmap, INTPAIR(x,y));
    if(cnk) {
        mutex_unlock(&chunklock);
        return cnk;
    }
    cnk= init_chunk(x,y);
    hashmap_insert(chunkmap, INTPAIR(x,y),cnk);
    mutex_unlock(&chunklock);
    return cnk;
}
