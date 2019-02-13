#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

typedef struct
{
    uint32_t *buffer;
    uint32_t size;
    
    uint32_t *head;
    uint32_t *tail;
    
    mutex_t *mut;
    semaphore_t *read_sema;
    semaphore_t *write_sema;
    
} message_queue_t;

void message_queue_init(
    message_queue_t *const queue,
    uint32_t *buffer, uint32_t length,
    mutex_t *const mut,
    semaphore_t *const read_sema,
    semaphore_t *const write_sema
);

void message_queue_add(message_queue_t *const queue, uint32_t value);

uint32_t message_queue_get(message_queue_t *const queue);


#endif /* MESSAGE_QUEUE_H_ */

