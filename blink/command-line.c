#include "config.h"

/* ---------------------------------------------------------
   define a simple command to demonstrate command-line 
   
   command: ID
   param:   none
   output:  firmware version
   time:    0
   depends: none

   your command-line function must be named
     cmd_<YOUR FUNCTION NAME>
   you must declare a help message for your func:
     HELP_MSG(<YOUR FUNCTION NAME>, "help text" )
*/
HELP_MSG(ID, "ID - write firmware name and version" );

void cmd_ID(void)
{
    writeln("ID blinki 12 2");
}


/* ---------------------------------------------------------
   change speed of blinking led 
   the speed gives the number of on/off cycles per second
   the time between status change is calculated via:
      delay = 500ms / SPEED
 
   command: SPEED
   param:   number 1-10 
   output:  current speed 1-10
   time:    0
   depends: tcb
   GLOBAL:  SPEED
*/
u8 SPEED=1;

void blinki_update_delay(void)
{
    u16 d = 30000;
    SBI_PORT(PIN_LED);
    if( SPEED==0) d=0; else d /= SPEED;
    tcb_delay( BLINK_CB, d );
}

HELP_MSG(SPEED, "SPEED <0-200> --  set blinks per minute" );
void cmd_SPEED(void)
{    
    u8 s,p=6;
    if( LN.buf[p-1] != 32 ) goto output_speed;
    if( cl_num8(&p,&s) || s>200 ) {
	writeln("ERR SPEED parse arg: %s", LN.buf);
	return;
    }
    SPEED=s;
    blinki_update_delay();
 output_speed:
    writeln("SPEED %d", SPEED);
}



/* ---------------------------------------------------------
   initialize command-line array with defined commands 
*/
const cmd_t cl[] PROGMEM =
    {
     CMD(help), /* defined in cmdln.c */
     CMD(SPEED),
     CMD(ID)
    };
const u8 max_cmd = ALEN(cl);


/* ---------------------------------------------------------
   create comm interface, prefix is "log":
   PORT  : USART-0
   IO-BUF: 2^7 = 128
   functions: log_in_is_empty, log_in_get, log_put;
   REG_UBRR0, REG_UCSR0A must be defined in config.h
*/
GEN_SERX(log, 0, 128)
void command_line_init(void)
{
    log_init(REG_UBRR0);
    UCSR0A = REG_UCSR0A;
    xfunc_out = log_put;   // stdout for xputs,xprintf
    cmd_buf_empty = log_in_is_empty;   // cmdln interface
    cmd_buf_get = log_in_get;
}
/* ---------------------------------------------------------
 */
