#include "config.h"
#include "command-line.h"

#define KEYCODE(i) pgm_read_word( keys+i )
volatile u16 ir_cmd;
volatile u8  ir_rdy;
#define inr(a,b,c) (a>=b && a<+c)

static const PROGMEM u16 keys[] = {
				   0,0x8440,
				   1,0x5440,
				   2,0x9440,
				   3,0x1440,
				   4,0xE440,
				   5,0x6440,
				   6,0xA440,
				   7,0x2440,
				   8,0xC440,
				   9,0x4440,
				   0,0
};

void blink_cb(void)
{
  XBI_PORT(PIN_LED);
}

u16 convert_raw_to_key( u16 raw )
{
  u8 i=0;
  u16 code;
  while( (code=KEYCODE( i+1)) ) {
    if( code == raw ) {
      u16 key =  KEYCODE(i);
      LOG("%d", key );
      return key;
    }
    i+=2;
  }
  return 0;
}

static void emit_keycode(un32 dat)
{
  if( ir_rdy ) return;
  un16 cmd;
  cmd.b[0] = dat.b[2];
  cmd.b[1] = dat.b[0];
  ir_cmd=cmd.v;
  ir_rdy=1;
}

void ir_check_irq(void)
{
  static u8 bit_cnt;
  static un32 dat;
  static u16 tm0;
  static u8  hi;
  u8 lo;  
  u8 stat = GET_PIN(PIN_IRRECV0);

  // wait for signal going low, measure hi,lo pule width */
  lo = TIMER2 - tm0;
  tm0 = TIMER2;
  if( stat ) {
    hi = lo;
    return;
  }

  /* collect 32 data bits, reset on dirty pulse */ 
  if( inr(hi,3,8) ) {
    dat.v <<=1;
    /* bit 1 */ if( inr(lo,14,19) ) { dat.v |= 1; }
    /* error */ else if( ! inr(lo,3,8) ) { goto error; }   
    if( ++bit_cnt != 32 ) return; /* no errors until now */
    emit_keycode(dat);
  }

 error:
  bit_cnt=0;
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

	if( ir_rdy ) {
	  convert_raw_to_key(ir_cmd);	  
	  writeln("%04X", ir_cmd );
	  ir_rdy=0;
	}

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
