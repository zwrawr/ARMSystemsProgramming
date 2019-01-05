#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_

#include "os.h"

// How many tasks can this scheduler cope with?
#define PRIORITY_QUEUE_MAX_TASKS 16
#define AGEING_THRESHOLD 8

extern OS_Scheduler_t const priorityQueueScheduler;


#endif /* _PRIORITY_QUEUE_H_ */
