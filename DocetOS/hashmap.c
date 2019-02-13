#include "hashmap.h"
#include "stm32f4xx.h"
#include "simpleassert.h"

/*  The length of this array should be as large as the maximum number
*       of tasks you expect to have waiting at one time. proformace degrades when
*       the hash map starts to get full (often > 75% )
*   Prime numbers are benifical to the proformace of the hashing function.
*/
static OS_TCB_t *map[HASHMAP_SIZE] = {0};



/*static uint32_t hash(uint32_t key);*/
static uint32_t hashTCB(OS_TCB_t *task);


// Overload of hash for tcbs
static uint32_t hashTCB(OS_TCB_t *task)
{
    return hash(task->data);
}

// super simple hash function, but very fast
/*static*/ uint32_t hash(uint32_t key)
{

    // the point of the shift is that a modulo output only depends on the bits lower than its size.
    // Rotate and add may be better than this and multiplicative hashing would be better.
    return (
               ((uint32_t)(key) >> 4) ^
               (uint32_t)(key)
           ) % HASHMAP_SIZE;
}


/* Adds a tcb to the hash map */
void hashmap_add(OS_TCB_t *task)
{

    task->next = NULL;
    
    // get the hash of the task (task->data)
    uint32_t hashValue = hashTCB(task);
    
    
    if (map[hashValue] == NULL)
    {
        map[hashValue] = task; // if there is nothing in the bucket just fill it
    }
    else
    {
        // if there is already tasks in the bucket add this task to the list
        OS_TCB_t *current = map[hashValue];
        
        if (task == current)
        {
            return;
        }
        
				// get to the end of the list
        while ( current->next != NULL)
        {
        
            current = current->next;
            ASSERT(current != task);
            ASSERT(current != current->next);
        }
        
        if (task == current)
        {
            return;
        }
        
				// add the new task at the end
        current->next = task;
        
    }
    
}


/* Gets a tcb from the hash map
* potentially gets a list of tcbs or none
*/




// Dosen't care about hash collisions, means tasks may be woken from wait
// prematurly, but they will wait again when they next get ran
OS_TCB_t *simple_hash_remove(uint32_t key)
{
    uint32_t hashValue = hash(key);
    
		// if there are tasks in the bucket
    if (map[hashValue] != NULL)
    {
				
        OS_TCB_t *matches = map[hashValue]; // grab the tasks
        map[hashValue] = NULL; // clear the bucket
        
        return matches; // return the tasks with matching reasons
    }
    
    return NULL;
}

/* just because the hash matches it dosen't mean the data was the same.
* need to check each tasks data and build a new list of all the tasks to be returned
*
*   this lookes really inefficent but with an appropriatly sized array and a decent hash fucntion
* there should never be many tasks in each list
*/
OS_TCB_t *hashmap_remove(uint32_t key)
{
    uint32_t hashValue = hash(key); // hash the reason
    
    static int i = 0;
    
    if (map[hashValue] != NULL)
    {
        
        OS_TCB_t *matches = NULL;  // list of all of the tasks with matching data
        
        /*
        *   remove all the tasks whose data matches the key and add them to the matches  array
        */
        
        // use a dummy node to simplify the checks (no special case for the fist node)
        OS_TCB_t dummy;
        dummy.next = map[hashValue]; // start of the list for this hash bucket
        OS_TCB_t *curr = &dummy;
        
        // loop until the last element
        while (curr->next != NULL)
        {
        
            //ASSERT(curr != curr->next);
            
            if (curr->next->data == key)
            {
            
                OS_TCB_t *tmp = curr->next;  // get a refrence to the node were removing
                
                //ASSERT(curr != curr->next->next);
                if (curr->next->next != NULL)
                {
                    curr->next = curr->next->next; // unlink the next node that matches the key
                }
                else
                {
                    curr->next = NULL;
                }
                
                if (matches == NULL)
                {
                    matches = tmp; // adds the removed item to the matches list.
                    tmp->next = NULL; // removes the connection to the old list
                }
                else
                {
                    // adds the removed item to the front of the list
                    tmp->next = matches; // add matches to the end of tmp, this clears the connection to the previous list
                    matches = tmp; // make matches point to tmp
                }
            }
            else
            {
                //ASSERT(curr != curr->next);
                curr = curr->next; // move on to the next node
                
            }
        }
        
        map[hashValue] = dummy.next; // might not be needed since this is mainly done in place
        
        return matches;
    }
    
    return NULL;
    
}
