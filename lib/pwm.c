#include "pwm.h"

#define B_WGM13 (_BV(WGM13) * ((PWM1_MODE & 8)==8))
#define B_WGM12 (_BV(WGM12) * ((PWM1_MODE & 4)==4))
#define B_WGM11 (_BV(WGM11) * ((PWM1_MODE & 2)==2))
#define B_WGM10 (_BV(WGM10) * ((PWM1_MODE & 1)==1))

#define B_CS12 (_BV(CS12) * ((PWM1_PRESCALE & 4)==4))
#define B_CS11 (_BV(CS11) * ((PWM1_PRESCALE & 2)==2))
#define B_CS10 (_BV(CS10) * ((PWM1_PRESCALE & 1)==1))

#define B_COM1A1 (_BV(COM1A1) * ((PWM1_CHANNEL & 1)==1))
#define B_COM1A0 (_BV(COM1A0) * ((PWM1_CHANNEL & 1)==1))

#define B_COM1B1 (_BV(COM1B1) * ((PWM1_CHANNEL & 2)==2))
#define B_COM1B0 (_BV(COM1B0) * ((PWM1_CHANNEL & 2)==2))


 /**
 * @brief print values (LOG_ENABLE function)
 */
#if LOG_ENABLE > 0
u8 x_print(char* s, u8 n)
{
  xprintf(PSTR("%s %u\n"), s, n);
  return n;
}
#  define print(a,b) do { a=b; x_print(#a,b); } while(0)

#else
#  define print(a,b) a=b
#endif /* LOG_ENABLE */



/**
 * disable PWM hardware, release pins
 */
void pwm1_off(void)
{
  TCCR1A = 0;
}

/** 
 * set ocr1a pwm output
 */
void pwm1_out1(u16 top)
{
#if  (PWM1_CHANNEL & 1) > 0
    OCR1A = PWM1_TOP - top;
#endif
}

/**
 * set ocr1b pwm output
 */
void pwm1_out2(u16 top)
{
#if  (PWM1_CHANNEL & 2) > 0
    print( OCR1B, PWM1_TOP - top);
#endif
}

/**
 * Initialize PWM hardware
 */
void pwm1_init(void)
{
#if  (PWM1_CHANNEL & 1) > 0
    cfg_lo( PIN_PWM1A );
#endif
#if  (PWM1_CHANNEL & 2) > 0
    cfg_lo( PIN_PWM1B );
#endif

    print( ICR1H, PWM1_TOP / 256 );
    print( ICR1L, PWM1_TOP & 0xff );


    print( TCCR1A, B_COM1A1 | B_COM1A0 | B_COM1B1 | B_COM1B0 | B_WGM11 | B_WGM10);
    print( TCCR1B, B_WGM13 | B_WGM12 | B_CS12 | B_CS11 | B_CS10 );

    pwm1_out1( 0 );
    pwm1_out2( 0 );
}
