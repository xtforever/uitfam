#include "config.h"
#include "command-line.h"
#include "dht11.h"
#include "dig7x4.h"
#include "util/delay.h"


/*
 * Diese Funktion wird durch tcb_check aufgerufen (main_loop)
 * dazu wurde sie mit tcb_add() einer callback liste, die in
 * config.h definiert wird, hinzugefuegt
 * Zuerst wird die LED umgeschaltet
 * danach wird pin DHT11 auf low gelegt und mit tcb_delay
 * die zeit fuer den n√chsten Aufruf auf 20ms gesetzt.
 * nach 20ms wird der dht11 ausgelesen,
 * der status wird auf dem UART ausgegeben
 * das intervall bis zum n√chsten durchlauf wird zur√ckgesetzt.
 **/

   
void blink_cb(void)
{
	static u8 start_code = 1;

    	XBI_PORT(PIN_LED);

    	if( start_code ) {	
		start_code = 0;
		SBI_DDR(DHT11); 
		tcb_delay( BLINK_CB, 20 );
		return;
	}

	u8 e,f,t;
	start_code = 0;
	e=dht11(&f,&t);
    	blinki_update_delay();
    	writeln("%d %d %d", e,f,t);
    	if(e) return;
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
	dig7_update_cb();
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

    // tcb_add( DIG7_CB, dhtfetch, 1000 ); 
    dig7_num(1234);
    /* activate driver/irq */
    sei();

    /* give startup output and debug messages */
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    LOG("type 'help' for command line help");
    run(); /* never returns */
}

