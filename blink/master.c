#include "config.h"
#include "command-line.h"


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
   }    
}


int main(void)
{
    /* init hardware */
    cfg_lo(PIN_LED);
    
    /* init driver */
    timer2_init();
    cl_init();

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

