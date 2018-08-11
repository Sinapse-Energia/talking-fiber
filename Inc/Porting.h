#ifndef __LEVAUX_PORT_H
#define __LEVAUX_PORT_H

#if   defined(STM32L053xx)
    #include "stm32l0xx_hal.h"
    #include "Typedefs.h"
    #define DISABLE_INTERRUPT()       __disable_interrupt() 
    #define ENABLE_INTERRUPT()        __enable_interrupt()  
#elif   defined(STM32F405xx)
    #include "stm32f4xx_hal.h"
    #define DISABLE_INTERRUPT()       __disable_interrupt()
    #define ENABLE_INTERRUPT()        __enable_interrupt()
#elif   defined(STM32F427xx)
    #include "stm32f4xx_hal.h"
    #define DISABLE_INTERRUPT()       __disable_interrupt() 
    #define ENABLE_INTERRUPT()        __enable_interrupt()  
#elif defined(WIN32)
    #include "Typedefs.h"
    #define DISABLE_INTERRUPT()       
    #define ENABLE_INTERRUPT()     
    #define WAIT(x)          Sleep(x)
#elif defined(CPU_KE02)
    #include "Typedefs.h"
    #define DISABLE_INTERRUPT()       __disable_interrupt() 
    #define ENABLE_INTERRUPT()        __enable_interrupt()  
#else
    #error   "NOT SUPPORT CORE!!!!!!!!!!!!!"
#endif

#endif // __LEVAUX_PORT_H

