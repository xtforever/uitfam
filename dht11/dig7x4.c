#include "dig7x4.h"
#include <util/delay.h>

u8 disp_mem[4];

#define LATCH_PIN     D4
#define CLK_PIN       D7
#define DATA_PIN      B0


/* Segment byte maps for numbers 0 to 9 */
u8 SEGMENT_MAP_DIGIT[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
/* Segment byte maps for alpha a-z */
u8 SEGMENT_MAP_ALPHA[] = {136, 131, 167, 161, 134, 142, 144, 139 ,207, 241, 182, 199, 182, 171, 163, 140, 152, 175, 146, 135, 227, 182, 182, 182, 145, 182};

/* Byte maps to select digit 1 to 4 */
static u8 SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};


u8 AsciiToSegmentValue (u8 ascii)
{
  u8 segmentValue = 182;
  
  if (ascii >= '0' && ascii <= '9')
  {
    segmentValue = SEGMENT_MAP_DIGIT[ascii - '0'];
  }
  else if (ascii >= 'a' && ascii <='z')
  {
    segmentValue = SEGMENT_MAP_ALPHA[ascii - 'a'];
  }
  else if (ascii >= 'A' && ascii <='Z')
  {
    segmentValue = SEGMENT_MAP_ALPHA[ascii - 'A'];
  }
  else
  {
    switch (ascii)
    {
      case '-':
        segmentValue = 191;
        break;
      case '.':
        segmentValue = 127;
        break;
      case '_':
        segmentValue = 247;
        break;
      case ' ':
        segmentValue = 255;
        break;
    }
  }
  
  return segmentValue;
}

void dig7_init(void)
{
    cfg_hi( LATCH_PIN );
    cfg_hi( CLK_PIN );
    cfg_hi( DATA_PIN );
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
    seg &= 0x3;
    if( set )
	disp_mem[seg] &= ~128;
    else 
	disp_mem[seg] |= 128;
}

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

void sreg_print(u8 seg, u8 ascii)
{
    CBI_PORT( LATCH_PIN );
    sreg_write(ascii);
    sreg_write(SEGMENT_SELECT[seg]);
    SBI_PORT( LATCH_PIN );
}

void dig7_update_cb(void)
{
    static u8 disp_cnt;
    if( disp_cnt > 3 ) disp_cnt = 0;
    sreg_print(disp_cnt, disp_mem[disp_cnt] );
    /* set tcb_delay for intensity change */
    disp_cnt++;
}
