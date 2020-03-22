#include "rk-decode.h"



// encoder full-step state table
// emit a code at 00
#define DIR_NONE 0x0
#define R_START 0x0
#define R_CW_FINAL 0x1
#define R_CW_BEGIN 0x2
#define R_CW_NEXT 0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT 0x6

const unsigned char ttable[7][4] = {
  // R_START
  {R_START, R_CW_BEGIN, R_CCW_BEGIN, R_START},
  // R_CW_FINAL
  {R_CW_NEXT, R_START, R_CW_FINAL, R_START | DIR_CW},
  // R_CW_BEGIN
  {R_CW_NEXT, R_CW_BEGIN, R_START, R_START},
  // R_CW_NEXT
  {R_CW_NEXT, R_CW_BEGIN, R_CW_FINAL, R_START},
  // R_CCW_BEGIN
  {R_CCW_NEXT, R_START, R_CCW_BEGIN, R_START},
  // R_CCW_FINAL
  {R_CCW_NEXT, R_CCW_FINAL, R_START, R_START | DIR_CCW},
  // R_CCW_NEXT
  {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
};



u8 rk_decode(u8 pins)
{
    static u8 state = 0;
    state = ttable[state][pins & 0x03]; 
    return state;
}
