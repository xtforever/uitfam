/**
 to config pwm the generator you need some defines
 in your config.h:

 set desired pwm frequenz:
 #define PWM1_FREQ     10000UL

 enable pwm channel:
 #define PWM1_CHANNEL     ENABLE_OC1B

 set prescaler:
 #define PWM1_PRESCALE    PWM_PRESCALE_8

 set pwm mode:
 #define PWM1_MODE        PWM1_FASTPWM_TOP_ICR

 This will enable inverse PWM signal on Channel B, you can safely use pwm1_out1(0) to
 completly switch off your pwm signal, i.e. pin d4 is pulled low.
*/



#ifndef PWM_H
#define PWM_H

#include "config.h"

/* pin config ------------------- */
#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) 
#  define PIN_PWM1A B1
#  define PIN_PWM1B B2
#else
#  define PIN_PWM1A D5
#  define PIN_PWM1B D4
#endif

/* prescale --------------------- */
#define DISABLE_PRESCALER 1
#define PWM1_PRESCALE_8    2
#define PWM1_PRESCALE_64   3
#define PWM1_PRESCALE_256  4
#define PWM1_PRESCALE_1024 5

/* pwm mode and channel config -- */
#define PWM1_FASTPWM_TOP_ICR 14
#define ENABLE_OC1A      1
#define DISABLE_OC1A     0
#define ENABLE_OC1B      2
#define DISABLE_OC1B     0


void pwm1_init(void);    //< enable pwm, set ocr=0
void pwm1_off(void);    //< disable pwm, release pins
void pwm1_out1(u16 top); //< set duty cycle <PWM_TOP
void pwm1_out2(u16 top); //< set duty cycle <PWM_TOP



#if    PWM1_PRESCALE == DISABLE_PRESCALER
#define PWM1_DIV 1
#elif  PWM1_PRESCALE == PWM1_PRESCALE_8
#define PWM1_DIV 8
#elif  PWM1_PRESCALE == PWM1_PRESCALE_64
#define PWM1_DIV 64
#elif  PWM1_PRESCALE == PWM1_PRESCALE_256
#define PWM1_DIV 256
#elif  PWM1_PRESCALE == PWM1_PRESCALE_1024
#define PWM1_DIV 1024
#else
#error unknown pwm1 prescaler setting
#endif

#define PWM1_TOP ((F_CPU / PWM1_DIV / PWM1_FREQ) -1)
/**< calculate top counter value */

#if  PWM1_TOP < 10 || PWM1_TOP > 65535
#error wrong pwm1 prescaler or pwm1 freq setting
#endif



#endif
