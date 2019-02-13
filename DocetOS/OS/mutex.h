#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>
#include "task.h"

typedef struct
{
    volatile uint32_t counter;
    volatile OS_TCB_t *tcb;
} mutex_t;

void OS_mutex_init(mutex_t *mut);
void OS_mutex_acquire(mutex_t *mut);
void OS_mutex_release(mutex_t *mut);

#endif /* _MUTEX_H_ */
