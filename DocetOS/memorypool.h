#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

typedef struct
{
    void *array;
    void *head;
} memorypool_t;


#define MEMORY_POOL_SIZE 16

#endif /* MEMORY_POOL_H */
