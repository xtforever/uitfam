#ifndef UTIL_H
#define UTIL_H

#include "cpu.h"

void wait_ms(u16 ms);
void wait_seconds(u8 sec);

u8 wait_twi_ready(void);
u8 twi_read16( u8 id, u8 reg, u16 *ret );
u8 twi_read8( u8 id, u8 reg, u8 *ret );
u8 twi_enable(u8 mem, u8 len);
void twi_reset(void);

void lcd_hal_update(void);


void xdump(u8 *p, u8 len);
u8 parse16(char **s, u16 *d);
u8 parse8(char **s, u8 *d);
u8 getnum(u8 p);
#endif
