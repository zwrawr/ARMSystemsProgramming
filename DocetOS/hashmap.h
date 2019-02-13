#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "os.h"

// this number is prime and resonable for a system with 64 tasks
#define HASHMAP_SIZE 37

void hashmap_init(OS_TCB_t * * array, uint32_t length);
void hashmap_add(OS_TCB_t *task);
OS_TCB_t *hashmap_remove(uint32_t key);
OS_TCB_t *simple_hash_remove(uint32_t key);

uint32_t hash(uint32_t key);

#endif /* HASH_MAP_H */
