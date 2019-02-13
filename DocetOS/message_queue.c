#include "stm32f4xx.h"
#include "semaphore.h"
#include "mutex.h"
#include "message_queue.h"
#include "os.h"

void message_queue_init(
    message_queue_t *const queue,
    uint32_t *buffer, uint32_t length,
    mutex_t *const mut,
    semaphore_t *const read_sema,
    semaphore_t *const write_sema
)
{

    queue->buffer = buffer;
    queue->size = length;
    queue->mut = mut;
    queue->read_sema = read_sema;
    queue->write_sema = write_sema;
    queue->head = &(queue->buffer[0]);
    queue->tail = &(queue->buffer[0]);
    
}

void message_queue_add(message_queue_t *const queue, uint32_t value)
{

    if (queue != NULL )
    {
    
        // "get permission to write to the queue"
        semaphore_obtain(queue->write_sema);
        
        OS_mutex_acquire(queue->mut);
        *(queue->head) = value;
        
        if ( queue->head == (queue->buffer + queue->size))
        {
            // if we hit the end of the buffer go to the beggining
            queue->head = queue->buffer;
        }
        else
        {
            queue->head++;
        }
        
        // "give permission to read from the queue"
        semaphore_obtain(queue->read_sema);
        OS_mutex_release(queue->mut);
        
    }
    
    
}

uint32_t message_queue_get(message_queue_t *const queue)
{

    uint32_t dequeued = 0;
    
    if (queue != NULL)
    {
        // "get permission to read from the queue"
        semaphore_release(queue->read_sema);
        
        OS_mutex_acquire(queue->mut);
        
        dequeued = *(queue->tail);
        
        if ( queue->tail == (queue->buffer + queue->size))
        {
            // if we hit the end of the buffer go to the beggining
            queue->tail = queue->buffer;
        }
        else
        {
            queue->tail ++;
        }
        
        // "give permission to write from the queue"
        semaphore_release(queue->write_sema);
        
        OS_mutex_release(queue->mut);
        
    }
    
    return dequeued;
}
