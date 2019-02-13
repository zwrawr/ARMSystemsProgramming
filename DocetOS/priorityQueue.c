#include "priorityQueue.h"
#include "stm32f4xx.h"
#include "simpleassert.h"
#include <stdlib.h>
#include "hashmap.h"

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
static void priorityQueue_wait(void *const reason, uint32_t check);
static void priorityQueue_notify(void *const reason);


/*
*       HEAP for runnable tasks
*/
static uint32_t heapsize = 0;
static OS_TCB_t *runnable_tasks[PRIORITY_QUEUE_MAX_RUNNABLE] = {0};

static void removeTaskFromRunnable(OS_TCB_t *const tcb);
static void addTasktoRunnable(OS_TCB_t *const tcb);

static OS_TCB_t *extractMin(void);
static void minHeapify(uint32_t index);
static void decreaseKey(uint32_t i, uint32_t newPriority);
static void ageTasks (void);

/*
*       INSERTION SORTED LIST for sleeping tasks
*/

OS_TCB_t *sleeping_head = NULL;
static OS_TCB_t *removeHeadFromSleeping(void);

/* Scheduler block for the simple round-robin */
OS_Scheduler_t const priorityQueueScheduler =
{
    .preemptive = 1,
    .scheduler_callback = priorityQueue_scheduler,
    .addtask_callback = priorityQueue_addTask,
    .taskexit_callback = priorityQueue_taskExit,
    .wait_callback = priorityQueue_wait,
    .notify_callback = priorityQueue_notify
};




/* Round-robin scheduler callback */
static OS_TCB_t const *priorityQueue_scheduler(void)
{


    // run aging if required
    static int i = 0;
    
    if (++i % AGEING_THRESHOLD == 0)
    {
        ageTasks();
    }
    
    // check on sleeping tasks
    uint32_t time = OS_elapsedTicks();
    
    if (
        sleeping_head != NULL &&
        sleeping_head->data < time )
    {
    
        return removeHeadFromSleeping();
    }
    
    // Clear the yield flag if it's set - we simply don't care
    OS_currentTCB()->state &= ~TASK_STATE_YIELD;
    
    
    if ( heapsize > 0 )
    {
        OS_TCB_t *newTask = runnable_tasks[0];
        newTask->currentPriority = newTask->defaultPriority; // heapify???
        return newTask;
        
    }
    
    // No tasks in the list, so return the idle task
    return OS_idleTCB_p;
}


static void ageTasks ()
{
    for ( int i = 0; i < heapsize; i++)
    {
        if (runnable_tasks[i]->currentPriority > 0)
        {
            runnable_tasks[i]->currentPriority--;
        }
    }
    
    minHeapify(0);
    
}


/* 'Add task' callback */
static void priorityQueue_addTask(OS_TCB_t *const tcb)
{
    if (tcb != NULL)
    {
        addTasktoRunnable(tcb);
    }
}


static void addTasktoRunnable(OS_TCB_t *const tcb)
{
    // if the heap is full, simply discard the task
    if (heapsize == PRIORITY_QUEUE_MAX_RUNNABLE)
    {
        return;
    }
    
    runnable_tasks[heapsize] = tcb;
    int i = heapsize;
    heapsize++;
    
    while (
        i != 0 &&
        runnable_tasks[PARENT(i)]->currentPriority > runnable_tasks[i]->currentPriority
    )
    {
        SWAP(runnable_tasks[i], runnable_tasks[PARENT(i)], OS_TCB_t *);
        i = PARENT(i);
    }
}


static void decreaseKey(uint32_t i, uint32_t newPriority)
{

    runnable_tasks[i]->currentPriority = newPriority;
    
    while (
        i != 0 &&
        runnable_tasks[PARENT(i)]->currentPriority > runnable_tasks[i]->currentPriority
    )
    {
        SWAP(runnable_tasks[i], runnable_tasks[PARENT(i)], OS_TCB_t *);
        i = PARENT(i);
    }
}

/* 'Task exit' callback */
// TODO :: this can be optimised if only the running task can exit
static void priorityQueue_taskExit(OS_TCB_t *const tcb)
{
    /* this method can only be called by a task on its self.
    * This means the task has to be in the runnable queue
    * If tasks could end other tasks this method would need to be expanded
    */
    removeTaskFromRunnable(tcb);
}


static void removeTaskFromRunnable(OS_TCB_t *const tcb)
{
    for (int i = 0; i <= heapsize; i++)
    {
        if (runnable_tasks[i] == tcb)
        {
            decreaseKey(i, 0);
            extractMin();
            return;
        }
    }
    
}


static void removeCurrentFromRunnable()
{
    OS_TCB_t *currently_running =  OS_currentTCB();
    
    if (currently_running != runnable_tasks[0])
    {
        return;
    }
    
    ASSERT(currently_running == runnable_tasks[0]);
    extractMin();
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
        return runnable_tasks[0];
    }
    
    OS_TCB_t *root = runnable_tasks[0];
    runnable_tasks[0] = runnable_tasks[--heapsize];
    minHeapify(0);
    
    return root;
}

static void minHeapify(uint32_t index)
{
    uint32_t left_index = LEFT(index);
    uint32_t right_index = RIGHT(index);
    
    uint32_t smallest = index;
    
    if (left_index < heapsize &&
            runnable_tasks[left_index]->currentPriority < runnable_tasks[index]->currentPriority
       )
    {
        smallest = left_index;
    }
    
    if (right_index < heapsize &&
            runnable_tasks[right_index]->currentPriority < runnable_tasks[smallest]->currentPriority
       )
    {
        smallest = right_index;
    }
    
    if (smallest != index)
    {
        SWAP(runnable_tasks[index], runnable_tasks[smallest], OS_TCB_t *);
        minHeapify(smallest);
    }
}

/* 'Wait' callback */
static void priorityQueue_wait(void *const reason, uint32_t check)
{

    if (check != OS_checkValue())
    {
        return;
    }
    
    OS_currentTCB()->data = (uint32_t)reason;
    OS_currentTCB()->state |= TASK_STATE_WAIT;
    
    // add task to waiting
    hashmap_add(OS_currentTCB());
    
    // remove task from runnable
    removeCurrentFromRunnable();
    
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    
}

/* 'Notify' callback */
static void priorityQueue_notify(void *const reason)
{


    OS_TCB_t *removed = simple_hash_remove((uint32_t)reason);
    
    if (removed == NULL)
    {
        return;
    }
    
    // for every task that was removed
    do
    {
    
        // grab the task to be added
        OS_TCB_t *tmp = removed;
        
        // unlink it from list
        removed = removed->next;
        tmp->next = NULL;
        
        // clear the wait flag and data
        tmp->state &= ~TASK_STATE_WAIT;
        tmp->data = 0;
        
        // add it back to the runnable queue
        addTasktoRunnable(tmp);
        
    }
    while (removed != NULL);
    
    
    
    for (int i = 0; i < PRIORITY_QUEUE_MAX_RUNNABLE; i++)
    {
        if (runnable_tasks[i] != 0)   // for all tasks that are not null
        {
            if (runnable_tasks[i]->data == (uint32_t)reason)
            {
            
            
            }
        }
    }
}

static void addTaskToSleeping(OS_TCB_t *task)
{

    // if the tcb were adding to the list will wake up before the current head add it before the current head
    if ( sleeping_head == NULL ||  sleeping_head->data >= task->data)
    {
        task->next = sleeping_head;
        sleeping_head = task;
    }
    // if the tcb were adding is after the head
    else
    {
    
        OS_TCB_t *current = sleeping_head;  // grab a pointer to the head that we can use it iterate over the list
        
        while ( current->next != NULL &&
                current->next->data < task->data )
        {
        
            current = current->next;
        }
        
        task->next = current->next;
        current->next = task;
        
    }
    
}

/*static void removeTaskFromSleeping(OS_TCB_t * task) {

    if (*sleeping_head == task){
        // we are managing memory outside of this function
        *sleeping_head = (*sleeping_head)->next; // Oder of operations, is this valid
    } else
    {
        node * current = *sleeping_head;

        while (current->next != task) {
            current = current->next;
        }

        current->next = current->next->next;
        current->next=task;

    }
}*/

static OS_TCB_t *removeHeadFromSleeping()
{

    OS_TCB_t *removed = sleeping_head;  // grab a refrence to the task that is being removed
    
    sleeping_head = sleeping_head->next; // unlink the first node
    
    return removed;
}

void priorityQueue_sleep(uint32_t time)
{

    OS_TCB_t *task = OS_currentTCB();
    
    // setup task state
    task->data = OS_elapsedTicks() + time;
    task->state |= TASK_STATE_SLEEP;
    
    // put task in sleeping list
    addTaskToSleeping(task);
    
    // remove task from runnable
    removeCurrentFromRunnable();
    
    // initiate a task switch
    OS_yield();
    // returns here after sleep
    
    
    
    // remove from sleeping
    // task = removeHeadFromSleeping();
    
    // reset task state
    task->data = 0;
    task->state &= ~TASK_STATE_SLEEP;
    
    // add back to runnable
    addTasktoRunnable(task);
    
    
}
