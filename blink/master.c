#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h>

#include "cpu.h"
#include "xitoa.h"
#include "serx.h"
#include "timer2.h"
#include "cmdln.h"
#include "adc.h"
#include "tcb.h"

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

    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    run(); /* never returns */
}

