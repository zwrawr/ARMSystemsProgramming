#include "priorityQueue.h"

/* heap macros */
#define LEFT(i) (2*i +1) // calculates the index of the left child element
#define RIGHT(i) (2*i +2) // calaculates the index of the right child element
#define PARENT(i) ((i-1)/2) // calculate the index of the parent element

/* Macro to swap two vairables in line, with a do while guard */
#define SWAP(a, b, T) do { T SWAP_TEMP_VARIABLE = a; a = b; b = SWAP_TEMP_VARIABLE; } while (0)


/* Prototypes (functions are static, so these aren't in the header file) */
static OS_TCB_t const *priorityQueue_scheduler(void);
static void priorityQueue_addTask(OS_TCB_t *const tcb);
static void priorityQueue_taskExit(OS_TCB_t *const tcb);


static OS_TCB_t *tasks[PRIORITY_QUEUE_MAX_TASKS] = {0};



/* heap */
static OS_TCB_t *extractMin(void);
static void minHeapify(uint32_t index);
static void decreaseKey(uint32_t i, uint32_t newPriority);
static void ageTasks (void);

uint32_t heapsize;

/* Scheduler block for the simple round-robin */
OS_Scheduler_t const priorityQueueScheduler =
{
    .preemptive = 1,
    .scheduler_callback = priorityQueue_scheduler,
    .addtask_callback = priorityQueue_addTask,
    .taskexit_callback = priorityQueue_taskExit
};




/* Round-robin scheduler callback */
static OS_TCB_t const *priorityQueue_scheduler(void)
{


    static int i = 0;
    
    if (++i % AGEING_THRESHOLD == 0)
    {
        ageTasks();
    }
    
    
    // Clear the yield flag if it's set - we simply don't care
    OS_currentTCB()->state &= ~TASK_STATE_YIELD;
    
    
    if ( heapsize > 0 )
    {
        OS_TCB_t *newTask = tasks[0];
        newTask->currentPriority = newTask->defaultPriority;
        return newTask;
        
    }
    
    // No tasks in the list, so return the idle task
    return OS_idleTCB_p;
}


static void ageTasks ()
{
    for ( int i = 0; i < heapsize; i++)
    {
        if (tasks[i]->currentPriority > 0)
        {
            tasks[i]->currentPriority--;
        }
    }
    
    minHeapify(0);
    
}


/* 'Add task' callback */
static void priorityQueue_addTask(OS_TCB_t *const tcb)
{

    // if the heap is full, simply discard the task
    if (heapsize == PRIORITY_QUEUE_MAX_TASKS)
    {
        return;
    }
    
    tasks[heapsize] = tcb;
    int i = heapsize;
    heapsize++;
    
    while (
        i != 0 &&
        tasks[PARENT(i)]->currentPriority > tasks[i]->currentPriority
    )
    {
        SWAP(tasks[i], tasks[PARENT(i)], OS_TCB_t *);
        i = PARENT(i);
    }
}


static void decreaseKey(uint32_t i, uint32_t newPriority)
{

    tasks[i]->currentPriority = newPriority;
    
    while (
        i != 0 &&
        tasks[PARENT(i)]->currentPriority > tasks[i]->currentPriority
    )
    {
        SWAP(tasks[i], tasks[PARENT(i)], OS_TCB_t *);
        i = PARENT(i);
    }
}

/* 'Task exit' callback */
static void priorityQueue_taskExit(OS_TCB_t *const tcb)
{

    for (int i = 0; i <= heapsize; i++)
    {
        if (tasks[i] == tcb)
        {
            decreaseKey(i, 0);
            extractMin();
            return;
        }
    }
    
}


static OS_TCB_t *extractMin()
{
    if (heapsize <= 0) // of there is nothing in the heap return null
    {
        return NULL;
    }
    
    if (heapsize == 1)  // only one value so no need to rearrage heap
    {
        heapsize--;
        return tasks[0];
    }
    
    OS_TCB_t *root = tasks[0];
    tasks[0] = tasks[--heapsize];
    minHeapify(0);
    
    return root;
}

static void minHeapify(uint32_t index)
{
    uint32_t left_index = LEFT(index);
    uint32_t right_index = RIGHT(index);
    
    uint32_t smallest = index;
    
    if (left_index < heapsize &&
            tasks[left_index]->currentPriority < tasks[index]->currentPriority
       )
    {
        smallest = left_index;
    }
    
    if (right_index < heapsize &&
            tasks[right_index]->currentPriority < tasks[smallest]->currentPriority
       )
    {
        smallest = right_index;
    }
    
    if (smallest != index)
    {
        SWAP(tasks[index], tasks[smallest], OS_TCB_t *);
        minHeapify(smallest);
    }
}
