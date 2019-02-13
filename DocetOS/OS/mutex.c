#include "mutex.h"
#include "stm32f4xx.h"
#include "os.h"


void OS_mutex_init(mutex_t *mut)
{
    mut->counter = 0;
    mut->tcb = 0;
}

void OS_mutex_acquire(mutex_t *mut)
{

    while (1)
    {
        uint32_t tcb  = __LDREXW (&(mut->tcb));
        
        if (tcb == 0)
        {
            if ( __STREXW ((uint32_t)OS_currentTCB(), &(mut->tcb)) == 0)
            {
                // <---- Now in Mutex ---->
                break;
            }
        }
        else if (tcb == (uint32_t)OS_currentTCB())
        {
            // the current task already has the mutex
            break;
        }
        else
        {
            OS_wait(mut, OS_checkValue());
        }
    }
    
    mut->counter++;
    
}

void OS_mutex_release(mutex_t *mut)
{

    if ( OS_currentTCB() == mut->tcb )
    {
        mut->counter--;
        
        if (mut->counter == 0)
        {
            mut->tcb = 0;
            // <---- Now out of Mutex ----> // pontentially cloud be interupptted here. would cause a notify on a held mutex. not a big deal
            OS_notify(mut);
        }
    }
}

