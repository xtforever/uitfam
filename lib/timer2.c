#include "timer2.h"
#include <avr/interrupt.h>

volatile unsigned int TIMER2;

ISR(TIMER2_COMPA_vect)
{
    TIMER2++;
}

/**
* @brief Timer2 Interrupt starten
*
* MODE:   CTC
* PRESC:  256
* OC2:    62
* FCPU:   16Mhz
* ERROR:  0.8%
*
* FCPU:   20MHz
* OC2:    78
* ERROR:  0.1%
*
* FCPU:   8MHz
* OC2:    31
* ERROR:  0.8%
*
*/
void timer2_init(void)
{
  TCCR2A = _BV(WGM21);                 /* CTC */
  TCCR2B = _BV(CS22) | _BV(CS21);      /* 256 */
  OCR2A = (F_CPU / 256 / 1000);
  TCNT2 = 0;
  TIMSK2 = _BV(OCIE2A);
}


