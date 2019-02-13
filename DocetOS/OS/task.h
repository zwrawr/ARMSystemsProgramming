#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include <stddef.h>

/* Describes a single stack frame, as found at the top of the stack of a task
   that is not currently running.  Registers r0-r3, r12, lr, pc and psr are stacked
     automatically by the CPU on entry to handler mode.  Registers r4-r11 are subsequently
     stacked by the task switcher.  That's why the order is a bit weird. */
typedef struct s_StackFrame
{
    volatile uint32_t r4;
    volatile uint32_t r5;
    volatile uint32_t r6;
    volatile uint32_t r7;
    volatile uint32_t r8;
    volatile uint32_t r9;
    volatile uint32_t r10;
    volatile uint32_t r11;
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr;
    volatile uint32_t pc;
    volatile uint32_t psr;
} OS_StackFrame_t;

typedef struct OS_TCB_t OS_TCB_t;
struct OS_TCB_t
{
    /* Task stack pointer.  It's important that this is the first entry in the structure,
       so that a simple double-dereference of a TCB pointer yields a stack pointer. */
    void *volatile sp;
    /* This field is intended to describe the state of the thread - whether it's yielding,
       runnable, or whatever.  Only one bit of this field is currently defined (see the #define
       below), so you can use the remaining 31 bits for anything you like. */
    uint32_t volatile state;
    
    /* the prioority this task should start with and will return too once run ]
            lower values have more priority */
    uint_fast8_t volatile defaultPriority;
    
    /* the current priority of the task, may be lower than default if the task has aged. */
    uint_fast8_t volatile currentPriority;
    
    /* data stores intermediate data such as when a task wants to be woken up */
    uint32_t volatile data;
    
    /* pointer to next task in the chain */
    OS_TCB_t   *volatile next;
};

/* Constants that define bits in a thread's 'state' field. */
#define TASK_STATE_YIELD    (1UL << 0) // Bit zero is the 'yield' flag
#define TASK_STATE_SLEEP    (1UL << 1) // Bit one is the 'sleep' flag
#define TASK_STATE_WAIT    (1UL << 2) // Bit one is the 'wait' flag

#endif /* _TASK_H_ */
