#include "lcd_shield.h"
#include "lcd_hd44780.h"

/* lcd connected to PORTD (d4,d5,d6,d7)
   RS   : PB0
   EN   : PB1
*/

 
/* if lcd_command and lcd_write ram 
   follows to fast and display is
   corrupted or does not work at all
   define LCD_DELAY. 
*/
#undef LCD_VERY_SLOW

#define LCD_RS B0
#define LCD_EN B1

const u8 LCD_OFFS[ LCD_H-1 ] PROGMEM = LCD_OFFS_LIST;

/** write 4bit to display with rs 
    Bit 0..3 -> D4..D7, 
    Bit 4 == RS 
*/
void lcd_out(u8 d)
{
  
  if( d & 0x10 ) 
    SBI_PORT( LCD_RS );
  else
    CBI_PORT( LCD_RS );

  PORTD = (PORTD & 0x0f) | ((d & 0x0f) << 4);
  SBI_PORT( LCD_EN );
  _delay_us(1);
  CBI_PORT( LCD_EN );
  _delay_us(1);
}

/* Write 8bit LCD Command RS=0
 */
void lcd_command( u8 d )
{
  lcd_out( d>>4 ); 
  lcd_out( d & 0x0f ); 
  

  _delay_us(50);


}

// Write 8bit Data to Display (RS=1)
void lcd_write_ram(u8 d)
{
  u8 val;
  val = d >> 4;
  val |= 0x10; // RS=1
  lcd_out(val); 

  val = d & 0x0f;
  val |= 0x10; // RS=1
  lcd_out(val); 
#ifdef LCD_VERY_SLOW 
  _delay_us(50);
#endif
}

void lcd_init( void ) {
  cfg_lo( LCD_RS );
  cfg_lo( LCD_EN );
  DDRD |= 0xf0; /* d4,d5,d6,d7 Ausgang */
  
  lcd_hd44780_init(); 

  lcd_command( LCD_SET_DDADR  ); 
  _delay_ms(1);lcd_write_ram('G'); 
  _delay_ms(1); lcd_write_ram('O'); 

}
