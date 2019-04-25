#ifndef LCD_HAL_H
#define LCD_HAL_H

#include "cpu.h"
#include <util/delay.h>

#ifdef USE_CONFIG_H
#include "config.h"
#endif

// ---------------------------------------
// interface hardware 
// must be defined in config.h
// ---------------------------------------

#include "lcd_hd44780.h"

extern const u8 LCD_OFFS[ LCD_H -1 ];
void lcd_init(void);
void lcd_command(u8 ch);
void lcd_write_ram(u8 ch);

// ---------------------------------------
#if !defined( LCD_W ) || ! defined( LCD_H )
#error you must set LCD_W/LCD_H/LCD_OFFS in your config.h
#endif
// ---------------------------------------
// HAL lcd
// ---------------------------------------
struct LCD_CTRL_st {
  char scr[LCD_W * LCD_H];
  u8 crsr, dirty, upd;
};

extern struct LCD_CTRL_st LCD_CTRL;

void lcd_hal_init(void);
void lcd_putc( u8 ch );
void lcd_puts( const char *s );
void lcd_newline(void);
void lcd_pputs(const char *s );
void lcd_update(void);
void lcd_gotoxy( u8 x, u8 y );
void lcd_clr_eol(void); /* clear to end of line */
void lcd_write_int(long n, u8 w);
#endif
