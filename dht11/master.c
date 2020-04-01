#include "config.h"
#include "command-line.h"
#include "dht11.h"
#include "dig7x4.h"

void blink_cb(void)
{
    XBI_PORT(PIN_LED);
    u8 e,f,t;
    e=dht11(&f,&t);
    writeln("%d %d %d", e,f,t);
    dig7_num(f*100+t);
    dig7_dot(1,1);
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
    dig7_init();
    
    /* init driver */
    timer2_init();
    cl_init();

    /* init command-line interface - command-line.c */
    command_line_init();
    
    /* add timer callback for blinki */
    tcb_add( BLINK_CB, blink_cb, 600 ); 
    /* set default value for blink speed */
    blinki_update_delay();

    tcb_add( DIG7_CB, dig7_update_cb, 5 ); 
    dig7_num(1234);
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    LOG("type 'help' for command line help");
    run(); /* never returns */
}

