#include "config.h"
#include "command-line.h"

#define KEYCODE(i) pgm_read_word( keys+i )
volatile u16 ir_cmd;
volatile u8  ir_rdy;
#define inr(a,b,c) (a>=b && a<+c)

enum key_syms {
	       KEY_0,
	       KEY_1,
	       KEY_2,
	       KEY_3,
	       KEY_4,
	       KEY_5,
	       KEY_6,
	       KEY_7,
	       KEY_8,
	       KEY_9,
	       KEY_RIGHT,
};


static const PROGMEM u16 keys[] = {
				   0x8440,
				   0x5440,
				   0x9440,
				   0x1440,
				   0xE440,
				   0x6440,
				   0xA440,
				   0x2440,
				   0xC440,
				   0x4440,
				   0x5EB6,
				   0,0
};

void blink_cb(void)
{
  XBI_PORT(PIN_LED);
}

int8_t convert_raw_to_key( u16 raw )
{
  u8 i=0;
  u16 code;
  while( (code=KEYCODE(i)) ) {
    if( code == raw ) {
      LOG("%d", i );
      return i;
    }
    i++;
  }
  return -1;
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

  // wait for signal going low, measure hi,lo pulse width */
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


/* check if we got a message from the ir remote
   and start defined procedure 
*/
static void ir_check(void)
{
  if(! ir_rdy ) return;
  
  int8_t key = convert_raw_to_key(ir_cmd);
  if( key <0 ) writeln("? 0x%04X", ir_cmd );
  else writeln("%02X", key );
  ir_rdy=0;
}

void run(void)
{
  WDT_ENABLE();    
  while(1) {
    WDT_RESET();
    tcb_check();
    cl_parse();
    ir_check();
  }    
}


int main(void)
{
    /* init hardware */
    cfg_lo(PIN_LED);

    cfg_in_pullup(PIN_IRRECV0);
    PCICR |= _BV(PCIE2); /* enable PCINT2 pin-change int 2 == PORTD */
    PCMSK2 |= _BV(2);    /* PIN2 */
    
    /* init driver */
    timer2_init();
    cl_init();

    /* set resolution to 0.1ms, default 1ms */
    OCR2A = (F_CPU / 256 / 10000);

    /* init command-line interface - command-line.c */
    command_line_init();
    
    /* add timer callback for blinki */
    tcb_add( BLINK_CB, blink_cb, 0 ); 
    /* set default value for blink speed */
    blinki_update_delay();
    
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    run(); /* never returns */
}

