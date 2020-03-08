#include "command-line.h"
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
   initialize command-line array with defined commands 
*/
const cmd_t cl[] PROGMEM =
    {
     CMD(help), /* defined in cmdln.c */
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
