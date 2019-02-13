#include "memorypool.h"
#include "mutex.h"
#include "semaphore.h"

static mutex_t mut;
static semaphore_t read_sema;
static semaphore_t write_sema;

/* ===================================
*
*            100% unvalidated
*
* ==================================== */

// this code has not been fully tested, it probably has massive errors in it

void memorypool_init(memorypool_t *pool)
{
    // Initialise the pool
    pool->head = 0;
    
		// init the protections 
    OS_mutex_init(&mut);
    semaphore_init(&read_sema, MEMORY_POOL_SIZE);
    semaphore_init(&write_sema, MEMORY_POOL_SIZE);
}

// gets a chunk of memory
void *memorypool_allocate(memorypool_t *pool)
{

    if (pool == NULL)
    {
        return NULL;
    }
    
    // "get permission to remove from the pool"
    semaphore_obtain(&write_sema);
    
    OS_mutex_acquire(&mut);
    void *grabbed = pool->head;  // grab chunk
    void **temp = pool->head;  // grab next chunk
    pool->head = *temp; // relink to next chunk
    
    // "give permission to add back to the pool"
    semaphore_obtain(&read_sema);
    
    OS_mutex_release(&mut);
    
    
    return grabbed;
}

// returns a chunk of memory
void memorypool_deallocate(memorypool_t *pool, void *item)
{

    // "get permission to add back to the pool"
    semaphore_release(&write_sema);
    
    OS_mutex_acquire(&mut);
    void **temp = item;
    * temp = pool->head;
    pool->head = temp;
    
    // "give permission to remove from the pool"
    semaphore_release(&read_sema);
    
    OS_mutex_release(&mut);
    
}
