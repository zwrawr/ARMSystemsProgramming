#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

typedef struct
{
    volatile uint32_t tokens; // the current number of tokens this semaphore has
    uint32_t max; // The max number of tokens this semaphore has.
} semaphore_t;


void semaphore_init(semaphore_t *const sema, uint32_t max);

void semaphore_obtain(semaphore_t *const sema);

void semaphore_release(semaphore_t *const sema);


#endif /* SEMAPHORE_H_ */

