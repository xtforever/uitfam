#ifndef  CONFIG_H
#define  CONFIG_H

#include <avr/io.h>
#include <avr/wdt.h>
#include "cpu.h"
#include "xitoa.h"

#define WDT_RESET() wdt_reset()
#define WDT_ENABLE() wdt_enable(WDTO_500MS)
#define WDT_DISABLE() wdt_disable()

#define PROG_NAME "o2sens"

// #define BAUD 19200
// #define REG_UBRR0 25

#define BAUD 500000
#define REG_UBRR0 BAUD_TO_REG(BAUD)
#define REG_UCSR0A 0
/*
#define REG_UBRR0 BAUD_TO_REG_DOUBLE_MCK(BAUD)
#define REG_UCSR0A _BV(U2X0)
*/


/* twi wiring (arduino)
   SCL - A5       C5
   SDA - A4       C4
*/

/* schedule */
// #define STACK_SIZE 100
// #define NUM_TASKS 2

/* Blinkenlights / arduino  */
#define PIN_LED B5

/* twi config */
#define TWI_TIMEOUT   250
#define TWI1_BITRATE  10000UL
#define TWI_ID        15
#define twi_ptr_max   100


extern volatile u8 TWMEM[twi_ptr_max];

#define MAX_CALLBACK 1
#define STEPPER_CB 0


/* lcd init */
// #define LCD_W 20
// #define LCD_H 4
// #define LCD_CLK D3 
// #define LCD_DATA D2 
// #define LCD_2WIRE
// #include "lcd_shield.h"


/* ADC */
#define ADC_VREF _BV(REFS0)
#define N_ADC_CHANNELS 2
#define CHANNEL0             (0 | ADC_VREF)
#define CHANNEL1             (1 | ADC_VREF)

/* PWM */
// #define PWM1_FREQ        10000
// #define PWM1_PRESCALE    1
// #define PWM1_CHANNEL     (ENABLE_OC1A | DISABLE_OC1B)
// #define PWM1_MODE        PWM1_FASTPWM_TOP_ICR
#endif

/* schedule */
#define STACK_SIZE 100
#define NUM_TASKS 2
