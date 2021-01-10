#include "config.h"
#include "command-line.h"


void blink_cb(void)
{
  XBI_PORT(PIN_LED);
}



volatile u8 irrecv0_stat;
volatile u8 tm;
volatile u16 to;
volatile u8 header;
volatile u16 bit;
volatile u8 count;


volatile u16 ir_cmd;
volatile u8  ir_rdy;

#define inr(a,b,c) (a>=b && a<+c)
void ir_check_irq(void)
{
  static u32 word;
  static u16 tm0;
  static u8  hi;
  u8 lo;
  

  u8 stat = GET_PIN(PIN_IRRECV0);
  lo = TIMER2 - tm0;
  tm0 = TIMER2;

  // wait for signal going low */
  if( stat ) {
    hi = diff;
    return;
  }


  /* ---   data bit 
   */ 
  if( inr(hi,3,8) ) {
    word <<=1;
    /* bit 1 */ if( inr(lo,14,19) ) { word |= 1; }
    /* error */ else if( ! inr(lo,3,8) ) { goto error; }

    if( ++count == 32 ) {
      un32 n32;
      n32.v = word;
      un16 cmd;
      cmd.b[0] = n32.b[2];
      cmd.b[1] = n32.b[0];
      if(! ir_rdy ) {
	ir_rdy=1;
	ir_cmd=cmd.v;
      }
      writeln("%02X%02X %04X", n32.b[0], n32.b[2], cmd.v );
    }
    return; 
  }

 error:
  count=0;
}



void ir_check(void)
{
  static u32 word;
  u16 diff;
  u8 d;
  static u16 tm0;
  static u8  hi;
  u8 lo;
  
  to++;
  u8 stat = GET_PIN(PIN_IRRECV0);
  if( stat == irrecv0_stat ) return;
  irrecv0_stat = stat;

  diff = TIMER2 - tm0;
  tm0 = TIMER2;
  
  // writeln("%c %u", stat ? '1' : '0', diff );

  if( stat ) {
    hi = diff;
    return;
  }
  lo = diff;

  write("%u %u ", hi, lo );

  /* ---  start code 
   */ 
  if( inr(hi,70,110) ) {
    count=0; word=0;
    //    if( inr(lo,32,60) ) xputc('s');
    //    else if( inr(lo,16,32) ) xputc('r');
    //    else xputc('?');
    return;
  }

  /* ---   data bit 
   */ 
  if( inr(hi,3,8) ) {
    /* bit 1 */ if( inr(lo,14,19) ) { count++; word  <<=1; word |= 1; }
    /* bit 0 */ else if( inr(lo,3,8) ) { count++; word <<=1; }
    /* error */ else { count=0; return; }
    if( count == 32 ) {
      un32 n32;
      n32.v = word;
      writeln("\n---- %0X %0X", n32.b[0], n32.b[2] );
    }
  }

  // xputc('+');

}



ISR(PCINT2_vect)
{
  ir_check_irq();
}


void run(void)
{
  
    WDT_ENABLE();    
    while(1) {
        WDT_RESET();
	tcb_check();
	cl_parse();
        // ir_check();
   }    
}


int main(void)
{
    /* init hardware */
    cfg_lo(PIN_LED);

    cfg_in_pullup(PIN_IRRECV0);
    PCICR |= _BV(PCIE2); /* PORTD */
    PCMSK2 |= _BV(2);    /* PIN2 */
    
    /* init driver */
    timer2_init();
    cl_init();

    /* set resolution to 0.1ms */
    OCR2A = (F_CPU / 256 / 10000);

    
    /* init command-line interface - command-line.c */
    command_line_init();
    
    /* add timer callback for blinki */
    tcb_add( BLINK_CB, blink_cb, 600 ); 
    /* set default value for blink speed */
    blinki_update_delay();
    
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    run(); /* never returns */
}

