#ifndef ADC_H
#define ADC_H

#include "config.h"

typedef struct ADC_IRQ_st {
  u16 val[8];
  u8 state; // !=0 - locked, values ok
} ADC_IRQ_t;

#define ADC_PRESCALE_128 7
#define ADC_PRESCALE_64  6
#define ADC_VDIFF 3

extern volatile ADC_IRQ_t ADC_IRQ;
extern volatile ADC_IRQ_t ADC_BUF;

void adc_init(void);

#endif
