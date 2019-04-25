#ifndef TIMER2_H
#define TIMER2_H
#include <avr/interrupt.h>

#include "config.h"
/* include MAX_CALLBACK */


extern volatile unsigned int TIMER2;

void timer2_init(void);
inline unsigned int timer2_elapsed_time( unsigned int t2 );
inline unsigned int timer2_get(void);

inline unsigned int timer2_get(void)
{
  cli();
  unsigned int t1 = TIMER2;
  sei();
  return t1;
}

inline unsigned int timer2_elapsed_time( unsigned int t2 )
{
    unsigned int t1 = timer2_get();
    return t1 - t2;
}

#endif
