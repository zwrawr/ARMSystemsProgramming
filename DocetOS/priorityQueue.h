#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include "os.h"

// How many tasks can this scheduler cope with?
#define PRIORITY_QUEUE_MAX_RUNNABLE 64
#define PRIORITY_QUEUE_MAX_SLEEPING 64
#define PRIORITY_QUEUE_MAX_WAITING 64


#define AGEING_THRESHOLD 8

extern OS_Scheduler_t const priorityQueueScheduler;

void priorityQueue_sleep(uint32_t time);


#endif /* _PRIORITY_QUEUE_H_ */
