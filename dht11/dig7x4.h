#ifndef DIG7X4_H
#define DIG7X4_H

#include "config.h"

void dig7_init(void);
void dig7_num(u16 num);
void dig7_update_cb(void);
void dig7_dot(u8 seg, u8 on);
u8 AsciiToSegmentValue (u8 ascii);


#endif
