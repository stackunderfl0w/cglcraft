#include "mutex.h"
void mutex_init(mutex_t *mutex) {
    mutex->locked = false;
}

void mutex_lock(mutex_t *mutex) {
    while (__atomic_exchange_n(&mutex->locked, true, __ATOMIC_ACQUIRE)) {
        asm("pause");
    }
}

void mutex_unlock(mutex_t *mutex) {
    __atomic_store_n(&mutex->locked, false, __ATOMIC_RELEASE);
}