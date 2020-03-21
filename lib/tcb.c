#include "tcb.h"
#include "timer2.h"


struct TIMER_CALLBACK TCB[MAX_CALLBACK];
    
void tcb_add(u8 num, void(*func)(void), u16 ms )
{
    TCB[num].func = func;
    TCB[num].ms   = ms;
    TCB[num].tm   = timer2_get();
}

void tcb_del(u8 num)
{
    TCB[num].ms = 0;
}

void tcb_check(void)
{
    for(u8 i=0; i<MAX_CALLBACK; i++ ) {
        if( TCB[i].func && TCB[i].ms && 
            timer2_elapsed_time(TCB[i].tm) > TCB[i].ms )
            {
                TCB[i].func();
                TCB[i].tm   = timer2_get();
            }            
        }        
}

void tcb_delay(u8 num, u16 delay)
{
    TCB[num].ms = delay;
}


