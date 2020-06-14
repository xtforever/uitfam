#include "config.h"
#include "command-line.h"
#include "dig7x4.h"
#include "util/delay.h"
#include "twi1.h"
volatile u8 TWMEM[twi_ptr_max];
#define RXBUF ((u8*)(TWMEM+2))


void blink_cb(void)
{
    //    static int num=0x3a;
    //    twi_read_from(0,num,0,0);
    //    LOG("read from: %u", num );
    //    num++;
    XBI_PORT(PIN_LED);
    // dig7_num(num);
}


void run(void)
{
    WDT_ENABLE();    
    while(1) {
        WDT_RESET();
	tcb_check();
	cl_parse();
	twhdl_exec();
	// dig7_update_cb();
    }    
}


int main(void)
{
    /* init hardware */
    cfg_lo(PIN_LED);
    // dig7_init();
    //    cfg_in_pullup( C4 ); cfg_in_pullup( C5 );
    
    /* init driver */
    timer2_init();
    cl_init();
    twi_init( TWI_ID, 1 );

    /* init command-line interface - command-line.c */
    command_line_init();
    
    /* add timer callback for blinki */
    tcb_add( BLINK_CB, blink_cb, 600 ); 

    /* set default value for blink speed */
    blinki_update_delay();

    /* init 4x7segment display on multifunction shield */
    // dig7_num(1234);
    
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    LOG("type 'help' for command line help");
    run(); /* never returns */
}

