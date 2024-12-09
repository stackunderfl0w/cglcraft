#pragma once 
#include <stdbool.h>

typedef struct {
    bool locked;
} mutex_t;

void mutex_init(mutex_t *mutex);

void mutex_lock(mutex_t *mutex);

void mutex_unlock(mutex_t *mutex);