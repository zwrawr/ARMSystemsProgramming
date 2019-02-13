#include "stm32f4xx.h"
#include "semaphore.h"
#include "os.h"

void semaphore_init(semaphore_t *const sema, uint32_t max)
{
    sema->tokens = sema->max = max;
}

void semaphore_obtain(semaphore_t *const sema)
{
    while (1)
    {
    
        uint32_t stored = 0;
        uint32_t tokenValue = __LDREXW (&(sema->tokens)); // load the current number of tokens
        
        if (tokenValue > 0)
        {
            tokenValue--; // take one
            stored = __STREXW (tokenValue, &(sema->tokens)); // update the number of tokens
            
            if (stored == 0)
            {
                // <-- success fully obtained a token, notify others about that
                OS_notify(sema);
                return;
            }
            
            // didn't get to update the number of tokens try again
        }
        else
        {
            // there were no tokens to get so wait, then try again
            OS_wait(sema, OS_checkValue());
        }
        
    }
}


void semaphore_release(semaphore_t *const sema)
{
    while (1)
    {
        uint32_t stored = 0;
        uint32_t tokenValue = __LDREXW (&(sema->tokens)); // get the number of tokens currently
        
        if (tokenValue < sema->max)
        {
            tokenValue++; // put one back
            stored =  __STREXW (tokenValue, &(sema->tokens)); // update the count of tokens
            
            if (stored == 0)
            {
                // successfully realesed a token, notify others about it
                OS_notify(sema);
                return;
            }
            
            // didn't manage to update the number of tokens, try again
            
        }
        else
        {
            // there was no room to release a token, wait and try again
            OS_wait(sema, OS_checkValue());
        }
    }
}
