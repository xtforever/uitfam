#include "config.h"
#include "command-line.h"
#include "rk-decode.h"

void rotary_knob_cb(void)
{
    static u8 switch_on =0;
    
    u8 fire = GET_PIN(RK_FIRE) == 0;    
    if( fire ^ switch_on ) {
	if( fire ) writeln( "#*" ); else writeln( "#-" );
	switch_on = fire;
    }

    u8 rk = (GET_PIN(RK_A) ? 1 : 0)
	| (GET_PIN(RK_B) ? 2 : 0);
    rk = rk_decode(rk);
    if( rk & DIR_CW )
	writeln( "#r" );
    else if( rk & DIR_CCW )
	writeln( "#l" );
    return;
}
			 
    
void blink_cb(void)
{
    XBI_PORT(PIN_LED);
}


void run(void)
{
    WDT_ENABLE();    
    while(1) {
        WDT_RESET();
	tcb_check();
	cl_parse();
	rotary_knob_cb();
   }    
}


int main(void)
{
    /* init hardware */
    cfg_lo(PIN_LED);
    cfg_in_pullup( RK_A );
    cfg_in_pullup( RK_B );
    cfg_in_pullup( RK_FIRE );
    
    /* init driver */
    timer2_init();
    cl_init();

    /* init command-line interface - command-line.c */
    command_line_init();
    
    /* add timer callback for blinki */
    tcb_add( BLINK_CB, blink_cb, 100 );
    /* set default value for blink speed */
    blinki_update_delay();
    
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    run(); /* never returns */
}

