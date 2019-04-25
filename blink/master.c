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


void blink_cb(void)
{
    XBI_PORT(PIN_LED);
}



/* ---------------------------------------------------------
   create comm interface, prefix is "log":
   PORT  : USART-0
   IO-BUF: 2^7 = 128
   functions: log_in_is_empty, log_in_get, log_put;
*/
GEN_SERX(log, 0, 128)
static void ser_init(void)
{
    log_init(REG_UBRR0);
    UCSR0A = REG_UCSR0A;
    xfunc_out = log_put;   // stdout for xputs,xprintf
    cmd_buf_empty = log_in_is_empty;   // cmdln interface
    cmd_buf_get = log_in_get;
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


HELP_MSG(ID, "ID - write firmware name and version" );
void cmd_ID(void)
{
    writeln("ID blinki 12 1");
}
  
const cmd_t cl[] PROGMEM = {
    CMD(help),
    CMD(ID)
};
const u8 max_cmd = ALEN(cl);

int main(void)
{
    cfg_lo(PIN_LED);
    ser_init();
    timer2_init();
    adc_init();
    cl_init();
    // cfg_in_pullup( C4 );
    // cfg_in_pullup( C5 );
    // tw_init( TWI_ID );

    tcb_add( BLINK_CB, blink_cb, 200 ); 
    sei();
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    run(); /* never returns */
}
