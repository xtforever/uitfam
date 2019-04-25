#ifndef TCB_H
#define TCB_H

#include "config.h"

struct TIMER_CALLBACK {
    void(*func)(void);
    u16 ms, tm;
};

extern struct TIMER_CALLBACK TCB[MAX_CALLBACK];

void tcb_add(u8 num, void(*func)(void), u16 ms );
void tcb_del(u8 num);
void tcb_check(void);
void tcb_delay(u8 num, u16 delay);

#endif
