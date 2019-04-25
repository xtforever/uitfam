#include "util.h"

#include "timer2.h"
#include "twi1.h"
#include "xitoa.h"
#include "schedule.h"
#include "lcd_hal.h"
#include "cmdln.h"

#ifndef LOG_ENABLE
#define LOG_ENABLE 0
#endif

void wait_ms(u16 ms)
{
    u16 tm = timer2_get();
    while( timer2_elapsed_time(tm) < ms ) schedule();
}

void wait_seconds(u8 sec)
{
    if( LOG_ENABLE ) xprintf(PSTR("wait for %u seconds... "), sec );
    u16 tm = timer2_get();
    while( timer2_elapsed_time(tm) / 1000 < sec )
        schedule();
    LOG("cont.");
}

u8 parse8(char **s, u8 *d)
{
    long u;
    if( xatoi(s,&u) && u >= 0 && u <= 0xff ) {
        *d = (u8) u;
        return 0;
    }
    return 1;
}

u8 parse16(char **s, u16 *d)
{
    long u;
    if( xatoi(s,&u) && u >= 0 && u <= 0xffff ) {
        *d = (u16) u;
        return 0;
    }
    return 1;
}

u8 getnum(u8 p)
{
  u8 num; char *s = LN.buf+p; 
  parse8(&s,&p); return p;
}


void xdump(u8 *p, u8 len)
{
    for(u8 i=0;;) {
        xprintf( PSTR("%02X"), p[i] );
        i++; if( i >= len ) break;
        xputc(32);
    }
}

/** twi reset */
void twi_reset(void)
{
    LOG("twi reset");
    tw_reset(); /* >100ms watchdog rst ! */
    wait_ms(10);
    TWI.stat=0;
}

/** warte bis der twi handler alle daten übertragen hat.
    tritt ein übertragungsfehler auf wird der twi controller
    zurückgesetzt.
    returns 0-twi ready, 1 - twi error, 2 - twi timeout
*/
u8 wait_twi_ready(void)
{
    while(1) {
        schedule();

        if( TWI.stat & TW_ERROR ) {
            LOG("twi error");
            twi_reset();
            return 1;
        }

        if(! (TWI.stat & TW_BUSY) ) return 0;

        if( timer2_elapsed_time(TWI.tm) > TWI_TIMEOUT ) {
            LOG("twi timeout");
            twi_reset();
            return 2;
        }
    }
}

/** 
 * read 16 bit from twi device id at register reg
 *
 */
u8 twi_read16( u8 id, u8 reg, u16 *ret )
{
    u8 err;
    twi_read_from(0, id, reg, 2 );
    if( (err=wait_twi_ready()) == 0 )
        {
            *ret = *(u16 *) (TWMEM+1);
        }
    return err;
}

/** 
 * read 8 bit from twi device id at register reg
 *
 */
u8 twi_read8( u8 id, u8 reg, u8 *ret )
{
    u8 err;
    twi_read_from(0, id, reg, 1 );
    if( (err=wait_twi_ready()) == 0 )
        {
            *ret = *(u8 *) (TWMEM+1);
        }
    return err;
}


/** enable tx/rx twi irq handler
 * input/output in array ( TWMEM[mem] - TWMEM[mem+len] )
 * returns 0-twi ready, 1 - twi error, 2 - twi timeout
*/
u8 twi_enable(u8 mem, u8 len)
{
    TWI.rw_start = mem;
    TWI.rw_max   = mem + len;
    twi_start();
    return wait_twi_ready();
}


/** update lcd display using scheduler 
 *
 * Update function for serial 2wire display with 20x4 Blue 
 * eadip204 type. should be called coninuously
 *
*/
void lcd_hal_update(void)
{
    if( ! LCD_CTRL.dirty ) return;
    u8 n,y,o; char *p = LCD_CTRL.scr;
    o=0;
    for(y=0;y<4;y++) {
        lcd_command(LCD_SET_DDADR + o);
        o+=0x20;
        wait_ms(1);
        for(n=0;n<LCD_W;n++) {
            lcd_write_ram( *p++ );
            wait_ms(1);
        }
    }
    LCD_CTRL.dirty=0;
}
