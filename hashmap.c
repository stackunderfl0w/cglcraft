#include "hashmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define LOAD_FACTOR 0.7

uint64_t fnv1a_hash(uint64_t key) {
    uint64_t hash = 14695981039346656037ULL;
    for (int i = 0; i < 8; i++) {
        hash ^= (key & 0xFF);
        hash *= 1099511628211ULL;
        key >>= 8;
    }
    return hash;
}

// Initialize hashmap
HashMap* hashmap_init() {
    HashMap *map = malloc(sizeof(HashMap));
    if (!map) return NULL;
    map->table = calloc(TABLE_SIZE, sizeof(HashEntry));
    if (!map->table) {
        free(map);
        return NULL;
    }
    map->size = 0;
    map->capacity = TABLE_SIZE;
    mutex_init(&map->lock);
    return map;
}

// Resize and rehash the hashmap
int hashmap_resize(HashMap *map) {
    size_t new_capacity = map->capacity * 2;
    HashEntry *new_table = calloc(new_capacity, sizeof(HashEntry));
    if (!new_table) return 0;

    // Rehash entries
    for (size_t i = 0; i < map->capacity; i++) {
        if (map->table[i].is_occupied) {
            uint64_t hash = fnv1a_hash(map->table[i].key) % new_capacity;
            size_t j = 0;
            while (new_table[(hash + j) % new_capacity].is_occupied) {
                j++;
            }
            new_table[(hash + j) % new_capacity] = map->table[i];
        }
    }

    free(map->table);
    map->table = new_table;
    map->capacity = new_capacity;
    return 1;
}

// Insert key-value pair into hashmap
void hashmap_insert(HashMap *map, uint64_t key, void *data) {
    mutex_lock(&map->lock);
    // Resize if load factor is exceeded
    if (map->size >= map->capacity * LOAD_FACTOR) {
        if (!hashmap_resize(map)) {
            return;  // Resize failed
        }
    }

    uint64_t hash = fnv1a_hash(key) % map->capacity;
    size_t j = 0;
    while (map->table[(hash + j) % map->capacity].is_occupied) {
        if (map->table[(hash + j) % map->capacity].key == key) {
            // Update existing key
            map->table[(hash + j) % map->capacity].data = data;
            mutex_unlock(&map->lock);
            return;
        }
        j++;
    }

    // Insert new key
    map->table[(hash + j) % map->capacity].key = key;
    map->table[(hash + j) % map->capacity].data = data;
    map->table[(hash + j) % map->capacity].is_occupied = 1;
    map->size++;
    mutex_unlock(&map->lock);
}

// Lookup data by key
void* hashmap_lookup(HashMap *map, uint64_t key) {
    mutex_lock(&map->lock);
    uint64_t hash = fnv1a_hash(key) % map->capacity;
    size_t j = 0;
    while (map->table[(hash + j) % map->capacity].is_occupied) {
        if (map->table[(hash + j) % map->capacity].key == key) {
            mutex_unlock(&map->lock);
            return map->table[(hash + j) % map->capacity].data;
        }
        j++;
    }
    mutex_unlock(&map->lock);
    return NULL;  // Key not found
}

// Free hashmap
void hashmap_free(HashMap *map) {
    free(map->table);
    free(map);
}
