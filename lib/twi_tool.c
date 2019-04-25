#include "twi1.h"
#include "twi_tool.h"
#include <avr/interrupt.h>






// returns 0 if twi is ready
u8 twi_check(void)
{
  // TWI Interrupt hat einen übertragungsfehler erkannt
  if (TWI.stat & TW_ERROR)
  {
  reset:
    LOG( "TWI %u %u", TWI.rx_start, TWI.stat );
    tw_reset();
    TWI.stat = 0;
    LOG("TWI RESET\n");
    return 1;
  }

  // falls ein time-out gestartet wurde, prüfen und ggf. TWI reset
  if (! (TWI.stat & TW_BUSY)) return 0;

  if (TWI.stat & TW_TMR_ON) {
      if (timer2_elapsed_time(TWI.tm) > TWI_TIMEOUT ) {
          LOG( "TWI TIMEOUT STAT=%02X\n", TWI.stat );
          goto reset;
      }
  }
  return 1;
}


// start twi timeout and twi master mode
void twi_start(void)
{
    TWI.tm = timer2_get();
    TWI.stat = TW_BUSY | TW_TMR_ON;
    TWCR = TW_ACK | _BV(TWSTA);
}

/** lesen von einem twi slave
    Ab der adresse (pos) werden (len) bytes vom slave gelesen,
    die Daten werden ab (bufp+1) gespeichert.
    Übertragung wird mit TWI_TIMEOUT überwacht
    bufp+0 wird für den twi header verwendet, also freilassen!
*/
void twi_read_from(u8 bufp, u8 id, u8 pos, u8 len)
{
  LOG( "TW RD\n" );
  u8* p = (void*) TWMEM + bufp;
  *p++ = id8(id << 1);
  *p = pos;
  TWI.rw_start = bufp;
  TWI.rw_max = bufp + len + 1;
  TWI.read_sw = bufp + 2;
  twi_start();
}


/** schreiben auf einem twi slave
    Ab der adresse (pos) werden (len) bytes geschrieben,
    die Daten werden ab (bufp+2) gelesen.
    bufp+0 und bufp+1 werden für den twi header benötigt
    Übertragung wird mit TWI_TIMEOUT überwacht

    Beispiel:
    RTC(twi id:17) erwartet die Uhrzeit in Register 50:

    aktuelle uhrzeit in TWMEM[ 2 ] .. TWMEM[12]
    twi_write_to( 0, 17, 50 10 );
*/
void twi_write_to(u8 bufp, u8 id, u8 pos, u8 len)
{
  LOG("TW WR\n");
  u8* p = (void*) TWMEM + bufp;
  *p++ = id8(id << 1);
  *p = pos;
  TWI.rw_start = bufp;
  TWI.rw_max = bufp + len + 2; // 2 BYTE HEADER
  twi_start();

}


/* twi handler and registry implementation
   jedes modul das den twi bus benutzen möchte,
   registriert einen handler mit
   twi_register()
   die nummer des handlers und die anzahl der handler werden
   in config.h definiert.

   sobald der twi-bus frei wird der nächste
   twi handler aufgerufen.
   Die resource-allokation erfolgt in drei schritten

   TWI_BUSY  - der Handler darf den TWI Bus weiterhin benutzen
   TWI_READY - der handler hat den TWI Bus freigegeben nächste
               funktion kann den bus reservieren


   TWI BUS Handler exmaple implementation

u8 handle_twi_message(u8 state)
{
    switch( state ) {
    case TWI_READY:
        TWMEM[0] = target_id;
        TWMEM[1] = cmd;
        TWI.rw_start = 0; TWI.rw_max = 3;
        twi_start();
        return TWI_BUSY;
    case TWI_BUSY:
        if( TWI.stat & TW_MT_RXC )
            module.sens = TWMEM[3];
        else
            module.sens = -1;
        return TWI_READY;
    }
}
*/


void twhdl_init(void)
{
    TWHDL.bus = TWI_READY;
    TWHDL.n = 0;
}

void twhdl_register(u8 n, twifunc_t fn)
{
    if( n < MAX_TWI_FUNC )
        TWHDL.fn[n] = fn;
}

static inline u8 next_twhdl(void)
{
    u8 i;
    for(i=0;i<MAX_TWI_FUNC;i++) {
        TWHDL.n++;
        if( TWHDL.n >= MAX_TWI_FUNC )  TWHDL.n=0;
        if( TWHDL.fn[TWHDL.n] ) return 0;
    }
    return 1; /* not found */
}

void twhdl_exec( void )
{
    if( twi_check() ) return; /* busy */

    if( TWHDL.bus == TWI_READY )
        if( next_twhdl() ) return; /* no hdl found */

    if( TWHDL.fn[TWHDL.n] == 0 ) return; /* hdl reg. error */

    TWHDL.bus = TWHDL.fn[TWHDL.n](TWHDL.bus);
};
