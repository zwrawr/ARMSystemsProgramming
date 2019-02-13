#ifndef SIMPLE_ASSERT_H
#define SIMPLE_ASSERT_H


#include "stm32f4xx.h" // provides __breakpoint()

#define ASSERT(x) do{if(!(x))__breakpoint(0);}while(0)

#endif /* SIMPLE_ASSERT_H */
