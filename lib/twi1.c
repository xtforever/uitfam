#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <avr/wdt.h>
#include "timer2.h"
#include "twi1.h"

#ifndef WDT_RESET
#define WDT_RESET() do {} while(0)
#endif

#define TW_TWBR ((F_CPU / TWI1_BITRATE - 16) / 2)
#define TW_ACK (_BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA))

u16 tw_reset_cnt = 0;

struct TWI_ST TWI __attribute__((section(".noinit")));

void tw_init(u8 addr)
{
  TWCR = 0; // this helps if twi is blocked after power on
  TWAR = id8(addr << 1);
  TWCR = TW_ACK;
  TWBR = (u8) TW_TWBR;
  memset(&TWI, 0, sizeof TWI);
  TWI.read_sw = twi_ptr_max + 1;
}

void tw_reset(void)
{
  TWCR = _BV(TWINT);
  _delay_ms(.1);
  WDT_RESET();
  TWDR = 0xff;
  TWCR = TW_ACK;
  _delay_ms(.1);
  WDT_RESET();
  TWCR = TW_ACK;
  TWI.stat = 0;
  tw_reset_cnt ++;
}


/** @todo TWI.ptr mit two.max vergleichen ob ALLE daten
 * übertragen wurden
 */
u8 tw_start(u8 start, u8 n)
{
  u8 retry = 8;
  u8 to, i;
  TWI.rw_start = start;
  TWI.rw_max = start + n;


  while (retry)
  {
    TWI.stat = TW_BUSY;
    TWCR = TW_ACK | _BV(TWSTA);

    to = 50;
    while (TWI.stat & TW_BUSY)
    {
      _delay_ms(0.1);
      to--;
      if (! to)
      {
        tw_reset();
        goto retry;
      }
    }

    if ((TWI.stat & (TW_BUSY | TW_ERROR)) == 0)
    {
      for (i = 0; i < 10; i++)
      {
        WDT_RESET();
        _delay_ms(1);
        WDT_RESET();
      }
      while (TWI.stat & TW_ERROR)
      {
        tw_reset();
        for (i = 0; i < 10; i++)
        {
          WDT_RESET();
          _delay_ms(1);
          WDT_RESET();
        }
      }
      return 0;
    }

  retry:
    retry--;
    if (TWI.stat & TW_ERROR) tw_reset();
  }

  return 1;
}


//
// transmit one byte,
// use repeated start to switch from master tx to master rx,
// receive n bytes store bytes at TWMEM[1..n]
u8 tw_read_at(u8 id, u8 p, u8 n)
{
  TWMEM[0] = id8(id << 1);
  TWMEM[1] = p;
  TWI.read_sw = 2; // after transmitting TWMEM[ TWI.read_sw-1 ] switch to Master receiver
  return tw_start(0, n + 1);
}

#define TWX (_BV(TWEN) | _BV(TWIE) | _BV(TWINT))

#ifdef TWI_LOG
u8 volatile twi_state[40];
u8 volatile twi_state_p = 0;
u8 get_twi_state(void)
{
  if( twi_state_p >= sizeof twi_state ) twi_state_p=0;
  return twi_state[twi_state_p++] = TW_STATUS;
}

#else
inline u8 get_twi_state(void);

inline u8 get_twi_state(void)
{
  return TW_STATUS;
}
#endif


ISR(TWI_vect)
{
  switch (last_twi_state = get_twi_state())
  {
  case TW_START:
  case TW_REP_START:
    TWI.ptr = TWI.rw_start; /* nächstes byte oder STOP senden */
  case TW_MT_DATA_ACK:
  case TW_MT_SLA_ACK:
    if (TWI.ptr == TWI.read_sw)
    { /* switch to Master Receiver */
      TWMEM[TWI.rw_start]++;
      TWI.read_sw = twi_ptr_max + 1; /* disable mode */
      TWCR = TWX | _BV(TWSTA); /* send repeated start */
      break;
    }
    if (TWI.ptr < TWI.rw_max)
    { /* continue? */
      TWDR = TWMEM[TWI.ptr++];
      TWCR = TWX;
    }
    else
    {
      register u8 stat = TWI.stat;
      stat &= ~ (TW_BUSY);
      stat |= TW_MT_TXC;
      TWI.stat = stat;
      TWCR = TWX | _BV(TWEA) | _BV(TWSTO ); /* stop */
    }
    break;

  case TW_MR_DATA_ACK:
    if (TWI.ptr < twi_ptr_max)
    TWMEM[TWI.ptr++] = TWDR;
    goto recv_next;

  case TW_MR_SLA_ACK:
    TWI.rx_start = TWI.ptr;
  recv_next:
    if (TWI.ptr < TWI.rw_max - 1)
      TWCR = TWX | _BV(TWEA);
    else
    {
      TWCR = TWX;
      TWI.stat |= TW_MT_RXC;
    }
    break;

  case TW_MR_DATA_NACK:
    if (TWI.ptr < twi_ptr_max)
    TWMEM[TWI.ptr++] = TWDR;
    TWI.stat &= ~ (TW_BUSY);
    TWCR = TWX | _BV(TWEA) | _BV(TWSTO ); /* stop */
    break;

  case TW_MT_ARB_LOST: /* also MR ARB LOST, retry */
    if (TWI.retry)
    {
      TWI.retry--;
      TWCR = TWX | _BV(TWSTA);
    }
    else
    {
      TWCR = TWX | _BV(TWEA);
      TWI.stat = TW_ERROR;
    }
    break;

  case TW_MR_SLA_NACK:
  case TW_MT_SLA_NACK:
  case TW_MT_DATA_NACK:
    TWCR = TWX | _BV(TWEA) | _BV(TWSTO ); /* stop */
    TWI.stat = TW_ERROR;
    break; // end_of_transmission;


  default:
  case TW_NO_INFO:
  case TW_BUS_ERROR:
  case TW_SR_DATA_NACK:
  case TW_SR_ARB_LOST_GCALL_ACK:
  case TW_SR_ARB_LOST_SLA_ACK:
    TWCR = TWX | _BV(TWEA);
    TWI.stat = TW_ERROR;
    break;

    // SLAVE RECEIVER
  case TW_SR_SLA_ACK: // addr ok
    TWI.ptr = 255;
    TWI.stat = TW_BUSY;
    TWCR = TWX | _BV(TWEA);
    break;
  case TW_SR_DATA_ACK: // receive data
    if (TWI.ptr == 255)
    {
      TWI.ptr = TWI.rx_start = TWDR; // start-adresse lesen
    }
    else
    {
      if (TWI.ptr < twi_ptr_max)
      TWMEM[TWI.ptr++] = TWDR;
    }
    TWCR = TWX | _BV(TWEA);
    break;

    // keine weiteren daten
  case TW_ST_DATA_NACK:
  case TW_ST_LAST_DATA:
    TWI.stat = TW_TX;
    TWCR = TWX | _BV(TWEA);
    break;
  case TW_SR_STOP:
    TWI.stat = TW_RX;
    TWCR = TWX | _BV(TWEA);
    break;

  case TW_ST_SLA_ACK:
    TWI.stat = TW_BUSY;
  case TW_ST_DATA_ACK:
  case TW_ST_ARB_LOST_SLA_ACK:
    if (TWI.ptr >= twi_ptr_max) TWI.ptr = 0;
    TWDR = TWMEM[TWI.ptr++];
    TWCR = TWX | _BV(TWEA);
    break;
  }
}


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

void twi_init(u8 id, u8 pullup_enable)
{
    if( pullup_enable ) {
	cfg_in_pullup( TWI_SDA );
	cfg_in_pullup( TWI_SCL );
    }
    tw_init(id);
}
