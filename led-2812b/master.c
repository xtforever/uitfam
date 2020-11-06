#include <avr/io.h>
#include <util/delay.h>
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
#include "schedule.h"
#include "util.h"


void dig7_update(void);


void ws2812_sendarray(uint8_t *data,uint16_t datlen);
void ws2812_sendarray_mask(uint8_t *, uint16_t , uint8_t);

/*
* simple WS2812 LED driver example 
* for Arduino Uno
*
* driver discussed and made by Tim in
* http://www.mikrocontroller.net/topic/292775
*
* addapted for Arduino Uno from 
*
* chris 30.7.2013
*
*/

// Data port register
#define ws2812_port PORTD 
// Number of the data out pin
#define ws2812_pin 5
// Number of LEDs
#define  ws2812_num 12
#define ARRAYLEN ((ws2812_num)*3)

// Color Led 
uint8_t ledArray[ARRAYLEN];
void ws2812_update(void)
{
    ws2812_sendarray(ledArray,ARRAYLEN);
}


static uint8_t ws2812b_state = 0;
void ws2812b_loop(void) 
{

    
  switch(ws2812b_state++) {
  default:
      ws2812b_state=0;
      // fallthrough, reset state
  case 0:
      ledArray[0]=0; 
      ledArray[1]=0;
      ledArray[2]=0; 
      break;
  case 1:
      ledArray[0]=255; 
      ledArray[1]=0;
      ledArray[2]=0; 
      break;
  case 2:
      ledArray[3]=0; 
      ledArray[4]=0;
      ledArray[5]=32; 
      break;
  case 3:
      ledArray[3]=32; 
      ledArray[4]=0;
      ledArray[5]=0; 
      break;
  case 4:
      memset(ledArray,0,ARRAYLEN);
      
      for(u8 i=0; i<ARRAYLEN; i++ ) {
	  ledArray[i] = 17;	  
      }
      break;
  case 5:
      memset(ledArray,0,ARRAYLEN);
      for(u8 i=0; i<ws2812_num; i++ ) {
	  ledArray[i*3 +2] = 75;	  
	  ledArray[i*3 +1] = 1;
	  ledArray[i*3   ] = 75;
      }
      break;
  case 6:
      memset(ledArray,0,ARRAYLEN);
      break;
  }
  ws2812_sendarray(ledArray,ARRAYLEN);
}

/*****************************************************************************************************************

    Led Driver Source from
    https://github.com/cpldcpu/light_ws2812/blob/master/light_ws2812_AVR/light_ws2812.c


     light weight WS2812 lib
    
     Created: 07.04.2013 15:57:49 - v0.1
     21.04.2013 15:57:49 - v0.2 - Added 12 Mhz code, cleanup
     07.05.2013 - v0.4 - size optimization, disable irq
     20.05.2013 - v0.5 - Fixed timing bug from size optimization
     27.05.2013 - v0.6 - Major update: Changed I/O Port access to byte writes
     30.6.2013 - V0.7 branch - bug fix in ws2812_sendarray_mask by chris
     
     instead of bit manipulation. This removes this timing
     discrepancy between standard AVR and reduced core so that
     only one routine is required. This comes at the cost of
     additional register usage.
     28.05.2013 - v0.7 - Optimized timing and size of 8 and 12 Mhz routines.
     All routines are within datasheet specs now, except of
     9.6 Mhz which is marginally off.
     03.06.2013 - v0.8 - 9.6 Mhz implementation now within specifications.
     - brvs->brcs. Loops terminate correctly
     Author: Tim (cpldcpu@gmail.com)

*****************************************************************************************************************/


void ws2812_sendarray(uint8_t *data,uint16_t datlen)
{
  ws2812_sendarray_mask(data,datlen,_BV(ws2812_pin));
}

/*
This routine writes an array of bytes with RGB values to the Dataout pin
using the fast 800kHz clockless WS2811/2812 protocol.
The description of the protocol in the datasheet is somewhat confusing and
it appears that some timing values have been rounded.
The order of the color-data is GRB 8:8:8. Serial data transmission begins
with the most significant bit in each byte.
The total length of each bit is 1.25µs (20 cycles @ 16Mhz)
* At 0µs the dataline is pulled high.
* To send a zero the dataline is pulled low after 0.375µs (6 cycles).
* To send a one the dataline is pulled low after 0.625µs (10 cycles).
After the entire bitstream has been written, the dataout pin has to remain low
for at least 50µs (reset condition).
Due to the loop overhead there is a slight timing error: The loop will execute
in 21 cycles for the last bit write. This does not cause any issues though,
as only the timing between the rising and the falling edge seems to be critical.
Some quick experiments have shown that the bitstream has to be delayed by
more than 3µs until it cannot be continued (3µs=48 cyles).

*/
void ws2812_sendarray_mask(uint8_t *da2wd3ta,uint16_t datlen,uint8_t maskhi)
{
  uint8_t curbyte,ctr,masklo;
  masklo	=~maskhi&ws2812_port;
  maskhi |=ws2812_port;
  cli();
  while (datlen--) 
  {
    curbyte=*da2wd3ta++; 
  
    asm volatile(
    " ldi %0,8 \n\t"	// 0
    "loop%=:out %2, %3 \n\t"	// 1
    " lsl %1 \n\t"	// 2
    " dec %0 \n\t"	// 3
    
    " rjmp .+0 \n\t"	// 5
    
    " brcs .+2 \n\t"	// 6l / 7h
    " out %2, %4 \n\t"	// 7l / -
    
    " rjmp .+0 \n\t"	// 9
    
    " nop \n\t"	// 10
    " out %2, %4 \n\t"	// 11
    " breq end%= \n\t"	// 12 nt. 13 taken
    
    " rjmp .+0 \n\t"	// 14
    " rjmp .+0 \n\t"	// 16
    " rjmp .+0 \n\t"	// 18
    " rjmp loop%= \n\t"	// 20
    "end%=: \n\t"
    :	"=&d" (ctr)
    :	"r" (curbyte), "I" (_SFR_IO_ADDR(ws2812_port)), "r" (maskhi), "r" (masklo)
    );
  }
  sei();
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



/* --------------------------------------------------------- */

#define BUTTON_1_PIN  C1
#define BUTTON_2_PIN  C2
#define BUTTON_3_PIN  C3

volatile u8 KEYS_PRESSED;

void keys_init(void)
{
    cfg_in_pullup( BUTTON_1_PIN );
    cfg_in_pullup( BUTTON_2_PIN );
    cfg_in_pullup( BUTTON_3_PIN );    
}



void keys_get( void )
{
    KEYS_PRESSED |= 7;
    
    if( GET_PIN(BUTTON_1_PIN) )
	KEYS_PRESSED ^= 1;
    if( GET_PIN(BUTTON_2_PIN) )
	KEYS_PRESSED ^= 2;
    if( GET_PIN(BUTTON_3_PIN) )
	KEYS_PRESSED ^= 4;
}

/* --------------------------------------------------------- */

void task_mainloop(void)
{
    WDT_ENABLE();    
    while(1) {
        WDT_RESET();
	tcb_check();
	cl_parse();
	schedule();
	keys_get();
	dig7_update();
    }    
}


HELP_MSG(ID, "ID - write firmware name and version" );
void cmd_ID(void)
{
    writeln("ID ws2812b_16Mhz_arduino 12 2");
}
  
const cmd_t cl[] PROGMEM = {
    CMD(help),
    CMD(ID)
};
const u8 max_cmd = ALEN(cl);


void set_led(u8 num, u8 r, u8 g, u8 b )
{
    
    while( num >= ws2812_num ) num -=  ws2812_num; 
    u8 *p = ledArray + (num*3);
    *p++ = g;
    *p++ = r;
    *p++ = b;
}




#define MAXPIX 42
#define COLORLENGTH (MAXPIX/2)
#define FADE (256/COLORLENGTH)

struct cRGB  { uint8_t g; uint8_t r; uint8_t b; };
struct cRGB colors[8];
struct cRGB led[MAXPIX];

void  rainbow(void)
{
	
	uint8_t j = 1;
	uint8_t k = 1;

	DDRB|=_BV(ws2812_pin);
		
    uint8_t i;
    for(i=MAXPIX; i>0; i--)
    {    
        led[i-1].r=0;led[i-1].g=0;led[i-1].b=0;
    }
		
    //Rainbowcolors
    colors[0].r=150; colors[0].g=150; colors[0].b=150;
    colors[1].r=255; colors[1].g=000; colors[1].b=000;//red
    colors[2].r=255; colors[2].g=100; colors[2].b=000;//orange
    colors[3].r=100; colors[3].g=255; colors[3].b=000;//yellow
    colors[4].r=000; colors[4].g=255; colors[4].b=000;//green
    colors[5].r=000; colors[5].g=100; colors[5].b=255;//light blue (türkis)
    colors[6].r=000; colors[6].g=000; colors[6].b=255;//blue
    colors[7].r=100; colors[7].g=000; colors[7].b=255;//violet
    
	while(1)
    {
        //shift all vallues by one led
        uint8_t i=0;           
        for(i=MAXPIX; i>1; i--) 
            led[i-1]=led[i-2];
        //change colour when colourlength is reached   
        if(k>COLORLENGTH)
        {
            j++;
            if(j>7)
            {
              j=0;
            }

            k=0;
        }
        k++;
        //loop colouers
        
        //fade red
        if(led[0].r<(colors[j].r-FADE))
            led[0].r+=FADE;
            
        if(led[0].r>(colors[j].r+FADE))
            led[0].r-=FADE;

        if(led[0].g<(colors[j].g-FADE))
            led[0].g+=FADE;
            
        if(led[0].g>(colors[j].g+FADE))
            led[0].g-=FADE;

        if(led[0].b<(colors[j].b-FADE))
            led[0].b+=FADE;
            
        if(led[0].b>(colors[j].b+FADE))
            led[0].b-=FADE;

		 wait_ms(50);
		 ws2812_sendarray((uint8_t *)led,MAXPIX*3);
    }
}



#define LATCH_PIN     D4
#define CLK_PIN       D7
#define DATA_PIN      B0



/* Segment byte maps for numbers 0 to 9 */
const u8 SEGMENT_MAP_DIGIT[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Segment byte maps for alpha a-z */
const u8 SEGMENT_MAP_ALPHA[] = {136, 131, 167, 161, 134, 142, 144, 139 ,207, 241, 182, 199, 182, 171, 163, 140, 152, 175, 146, 135, 227, 182, 182, 182, 145, 182};

/* Byte maps to select digit 1 to 4 */
const u8 SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};

static u8 disp_cnt = 0;
static u8 disp_mem[4] = { 0xF9,0xA4,0xB0,0x99 };



inline static void sreg_clock(void)
{
  _delay_us(2);
  SBI_PORT( CLK_PIN );
  _delay_us(2);
  CBI_PORT( CLK_PIN );
}

inline static void sreg_data(u8 d)
{
  if( d ) {
    SBI_PORT( DATA_PIN );
  }
  else {
    CBI_PORT( DATA_PIN );
  }
}

static void sreg_write( u8 d )
{
  u8 i;
  for(i=0;i<8;i++)
    {
      sreg_data( d & 0x80 );
      sreg_clock();
      d <<= 1;
    }
}

void dig7_print(u8 seg, u8 ascii)
{
    CBI_PORT( LATCH_PIN );
    sreg_write(ascii);
    sreg_write(SEGMENT_SELECT[seg]);
    SBI_PORT( LATCH_PIN );
}

void dig7_update(void)
{
    if( disp_cnt > 3 ) disp_cnt = 0;
    dig7_print(disp_cnt, disp_mem[disp_cnt] );
    /* set tcb_delay for intensity change */
    disp_cnt++;
}


void dig7_num(u16 num)
{
    u8 ch;
    u8 p;

    p=3;
    do {
	ch = num % 10;
	disp_mem[p] = SEGMENT_MAP_DIGIT[ch];
	num /= 10;
    } while( p-- && num);

    while( p < 4 ) {
	disp_mem[p] = 0xff;
	p--;
    }
}

void dig7_dot(u8 seg, u8 set)
{
}


void dig7_init(void)
{
    cfg_hi( LATCH_PIN );
    cfg_hi( CLK_PIN );
    cfg_hi( DATA_PIN );

    dig7_num( 1234 );
}




/* Einstellen der Farbe durch drei Taster

   Reagiere sofort
   Vermeide doppelte Betätigungen
   Erlaube schnell-vorlauf durch gedrückt halten

   1. Sobald eine Taste gedrückt wird, 
      wird die zugeordnete Funktion ausgeführt.
      Danach wird  150ms gewartet um doppelte
      Eingaben zu vermeiden
*/

volatile u8 COLOR[3];
void task_colorset(void)
{
    u8 index=0;

    while(1) {
	if( KEYS_PRESSED & 1 )
	    index++;
	if( index > 2 ) index=0;

	if( KEYS_PRESSED & 2 )
	    COLOR[index]+=10;

	if( KEYS_PRESSED & 4 )
	    COLOR[index] = 0;

	if( KEYS_PRESSED & 7 ) {
	    writeln("%d %d %d", COLOR[0], COLOR[1], COLOR[2] );

	    u16 n = (index+1) * 1000 + COLOR[index];
	    dig7_num(n);
	    disp_mem[0] &= ~128; /* make dot */
	    wait_ms(150);
	}
	schedule();

    }	
}


void task_blink(void)
{


    
    while(1) {

	// rainbow();
	// ws2812b_loop();
	// wait_ms(500);

	
	set_led(0, 250,0,0 );
	set_led(1, 227,23,10 );
	set_led(2, 200,75,30 );

	set_led(3,180,110,50 );
	set_led(4, 40,100,30 );
	set_led(5,90,200,70 );

	set_led(6, 80,235,80 );
	set_led(7, 20,250,90 );
	set_led(8, 10,250,100 );

	set_led(9, 20,10,200 );
	set_led(10, 10,0,230 );
	set_led(11, 150,10,250);
	ws2812_update();
	
	wait_ms(1000);
	
	writeln("update");
	
	u8 i=0;
	while(1) {
	    set_led(i, 0,0,0 );
	    set_led(i+1, COLOR[0], COLOR[1], COLOR[2] );
	    ws2812_update();
	    wait_ms(500);
	    i++;
	    if( i>=ws2812_num ) i=0;
	}	
    }
    
}





int main(void)
{
    keys_init();
    dig7_init();
    
    cfg_lo(D5);
    cfg_lo(PIN_LED);
    ser_init();
    timer2_init();
    adc_init();
    cl_init();

    rainbow();

    // cfg_in_pullup( C4 );
    // cfg_in_pullup( C5 );
    // tw_init( TWI_ID );
    // tcb_add( BLINK_CB, ws2812b_loop, 500 );

    sei();
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");

    task_create(task_mainloop);
    task_create(task_blink);
    task_create(task_colorset);
    
    run(); /* never returns */
}
