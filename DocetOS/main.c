#include "os.h"
#include <stdio.h>
#include "utils/serial.h"
//#include "simpleRoundRobin.h"
#include "priorityQueue.h"
#include "mutex.h"
#include "semaphore.h"
#include "message_queue.h"
#include "hashmap.h"

static mutex_t printf_mut; // mutex for control of the serial output
static message_queue_t queue; // message queue between task 4 and 5


static uint32_t isPrime(uint32_t prime);

// super basic prime check so that a task can be doing some proccessing
static uint32_t isPrime(uint32_t prime)
{
    if (prime == 1)
    {
        return 0;
    }
    else if (prime == 2 || prime == 3)
    {
        return 1;
    }
    else if (prime % 2 == 0 || prime % 3 == 0)
    {
        return 0;
    }
    else
    {
        uint32_t i = 5;
        
        while (i * i <= prime)
        {
            if (prime % i == 0 || prime % (i + 2) == 0 )
            {
                return 0;
            }
            
            i = i + 6;
        }
        
        return 1;
    }
}

/* This tasks demonstrates a task that is doing some data proccessing and is then outputting
* it's results
*/
void task1(void const *const args)
{

    uint32_t prime = 1;
    
    // print primes !!
    while (1)
    {
    
        OS_mutex_acquire(&printf_mut);
        printf("Task 1 : %d \r\n", prime);
        OS_mutex_release(&printf_mut);
        
        do
        {
            prime++;
        }
        while (!isPrime(prime));
        
    }
}

/* This task demonstates a task that outputs on a timed schedual
* I know this is not an acurate second, it's just for demo
*/
void task2(void const *const args)
{
    uint32_t secs = 0;
    
    while (1)
    {
        OS_mutex_acquire(&printf_mut);
        printf("\t\t%d SECONDS HAVE PASSED\r\n", secs);
        OS_mutex_release(&printf_mut);
        priorityQueue_sleep(1000);
        secs ++;
    }
}

/* This task demonstates a task that outputs on a timed schedual
* I know this is not an acurate minute, it's just for demo
*/void task3(void const *const args)
{
    uint32_t mins = 0;
    
    while (1)
    {
        OS_mutex_acquire(&printf_mut);
        printf("\t\t\t\t%d MINUTES HAVE PASSED\r\n", mins);
        OS_mutex_release(&printf_mut);
        priorityQueue_sleep(60000);
        mins ++;
    }
}

/* This task demonstates filling a inter-task message queue
*/
void task4 (void const *const args)
{

    for ( int i = 1; i < 10; i++ )
    {
        message_queue_add(&queue, i);
        OS_mutex_acquire(&printf_mut);
        printf("ADD %d \r\n", i);
        OS_mutex_release(&printf_mut);
        
    }
}

/* This task demonstates emptying a inter-task message queue
*/
void task5 (void const *const args)
{

    for ( int i = 1; i < 10; i++ )
    {
        uint32_t got = message_queue_get(&queue);
        OS_mutex_acquire(&printf_mut);
        printf("\t%d GET\r\n", got);
        OS_mutex_release(&printf_mut);
    }
}


/* MAIN FUNCTION */

int main(void)
{
    /* Initialise the serial port so printf() works */
    serial_init();
    
    printf("\r\nDocetOS Sleep and Mutex\r\n");
    
    
    /* Reserve memory for 5 stacks and 5 TCBs.
       Remember that stacks must be 8-byte aligned. */
    __align(8)
    static uint32_t stack1[64], stack2[64], stack3[64], stack4[64], stack5[64];
    static OS_TCB_t TCB1, TCB2, TCB3, TCB4, TCB5;
    
    
    // init the serial output mutex
    OS_mutex_init(&printf_mut);
    
    // statically define all the queue stuff
    static const uint32_t buf_size = 8;
    static uint32_t buffer[buf_size + 1];
    static mutex_t queueMutex;
    OS_mutex_init(&queueMutex);
    static semaphore_t rsema;
    static semaphore_t wsema;
    semaphore_init(&rsema, buf_size);
    semaphore_init(&wsema, buf_size);
    
    // init the message queue between task 4 and 5
    message_queue_init(&queue, buffer, buf_size, &queueMutex, &rsema, &wsema );
    
    
    /* Initialise the TCBs using the two functions above */
    OS_initialiseTCB(&TCB1, stack1 + 64, 16, task1, 0);
    OS_initialiseTCB(&TCB2, stack2 + 64, 4, task2, 0);
    OS_initialiseTCB(&TCB3, stack3 + 64, 3, task3, 0);
    OS_initialiseTCB(&TCB4, stack4 + 64, 1, task4, 0);
    OS_initialiseTCB(&TCB5, stack5 + 64, 2, task5, 0);
    
    /* Initialise and start the OS */
    OS_init(&priorityQueueScheduler);
    OS_addTask(&TCB5);
    OS_addTask(&TCB4);
    OS_addTask(&TCB3);
    OS_addTask(&TCB2);
    OS_addTask(&TCB1);
    OS_start();
}
