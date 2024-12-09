#pragma once
#include <stdint.h>
#include "mutex.h"

#define TABLE_SIZE 1024

typedef struct HashEntry {
    uint64_t key;
    void *data;
    int is_occupied;
} HashEntry;

typedef struct HashMap {
    HashEntry *table;
    uint32_t size;
    uint32_t capacity;
    mutex_t lock;
} HashMap;



HashMap* hashmap_init();
void hashmap_insert(HashMap *map, uint64_t key, void *data);
void* hashmap_lookup(HashMap *map, uint64_t key);
void hashmap_free(HashMap *map);