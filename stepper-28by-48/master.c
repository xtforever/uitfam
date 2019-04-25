/*

example code for stepper motor driver 
written by jens harms, au1064 (at) gmail (dot) com 
Date: 23.04.2019

This example needs the uitfam environment.  uitfam stands for
universal tasker for avr mcu and is a cooperative multi-tasking
library for avr microcontroller.  cooperative means, that you have to
call schedule() whenever your task needs to wait. For an example on
how this can be implemented, look at the function t_wait defined
below.
 

28byj-48

B1   MOT1    =   IN1   (blue)
B2   MOT2    =   IN2   (pink)
B3   MOT3    =   IN3   (yellow)
B4   MOT4    =   IN4   (orange)

half step or full step supported

1) coop-multitasking
2) cmdline interpreter
3) i2c comm
4) 

*/
#define MOT1   B1
#define MOT2   B2
#define MOT3   B3
#define MOT4   B4


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h>

/* uitfam includes */
#include "cpu.h"
#include "xitoa.h"
#include "serx.h"
#include "timer2.h"
#include "twi1.h"
#include "cmdln.h"
#include "adc.h"
#include "tcb.h"
#include "schedule.h"




/* current position of stepper motor */
static u16 stepper_pos = 0;

/* number of steps clockwise or counter clockwise to be taken in the background */
static int step_bg = 0;

/* change to #define FULL_STEP 0 if you want half step mode */ 
#define FULL_STEP 1 


static u8 step_cur = 0;
#if defined FULL_STEP > 0
static u8 step_pat[4] = { 0x36, 0xc9, 0x36,0xc9 };
static const u16 steps_per_round = 2048;
#else
static u8 step_pat[4] = { 0x13, 0x26, 0x4c, 0x89 };
static const u16 steps_per_round = 4096;
#endif

/* array for incomming and outgoing twi messages */
volatile u8 TWMEM[twi_ptr_max];

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

/* helper function for cooperative scheduler, wait for <ms> milliseconds
   and call schedule inside wait-loop to let other processes run.
*/
void t_wait(u16 ms)
{
    u16 t = timer2_get();
    while( timer2_elapsed_time(t) < ms ) schedule();
}


/* two helper functions for debugging only. */
#ifdef DEBUG_TRACE
static void print4(u8 x)
{
    u8 i;
    for(i=0;i<4;i++)
	{
	    if( x & 1 ) xputc('1'); else xputc('0');
	    x >>= 1;
	}
    xputc(10);
}

static void print_port(void)
{
    if( GET_PORT( MOT4 ) ) xputc(42); else xputc('.');
    if( GET_PORT( MOT3 ) ) xputc(42); else xputc('.');
    if( GET_PORT( MOT2 ) ) xputc(42); else xputc('.');
    if( GET_PORT( MOT1 ) ) xputc(42); else xputc('.');
    xputc(10);
}
#endif


static void step_init(void)
{
    cfg_lo( MOT1 );
    cfg_lo( MOT2 );
    cfg_lo( MOT3 );
    cfg_lo( MOT4 );
}

static void step_do(void)
{
    static u8 step_last = 0;
    u8 x,out;
    x= step_pat[step_cur/2];
    if(! (step_cur & 1) ) x >>= 4;   

    out = x ^ step_last;
    step_last = x;
    x = out;
    if( x & 1 ) XBI_PORT( MOT4 );
    if( x & 2 ) XBI_PORT( MOT3 );
    if( x & 4 ) XBI_PORT( MOT2 );
    if( x & 8 ) XBI_PORT( MOT1 );
}



/* command line interface helper function, parse a signed decimal integer number */
u8 cl_inum16(u8 *p, int *num)
{
    u8 sgn;
    if( LN.buf[*p] == '-' ) { (*p)++; sgn=1; }
    if( cl_num16(p, (u16*)num) ) return 1;
    if( sgn ) (*num)=-(*num);
    return 0;
}


/* make stepper turn to an absolute postion given in tenth degree */
HELP_MSG(turn, "turn <pos> : pos in degree * 10, ex: pos=1800 = 180°");
void cmd_turn(void)
{
    u16 pos;
    u8 p=5;
    if( cl_num16( &p, &pos ) ) return;

    // 3600 degree == 4096 step
    // 4096 / 3600 * d == step-pos2
    long tmp = steps_per_round;
    tmp *= pos;
    tmp /=  3600;
    step_bg = tmp;
    step_bg -= stepper_pos;

    /* falls mehr als 2048 steps laufe in die andere richtung 
       
       pos   = 4090
       new   =   90
       diff  =-4000
       opt   = 4096 + diff == 96;

       pos   =   10
       new   = 4090
       diff  = 4080
       opt   = -4096 + diff == -16
    */

    int half = steps_per_round / 2;
    if( step_bg < -half ) step_bg = steps_per_round + step_bg;
    if( step_bg > half  ) step_bg = step_bg  - steps_per_round;
   
    writeln("X TURN=%d", step_bg );
    writeln( "X CUR=%u", stepper_pos );
}


static void step_dir(u8 dir)
{
    step_do();
    if( dir ) {
	step_cur++;
	stepper_pos++;
    }
    else {
	step_cur--;
	stepper_pos--;
    }

    stepper_pos &= (steps_per_round-1);
    step_cur &= 0x07;
    t_wait(2);

}

void task_stepper(void)
{
    while(1) {
	if( step_bg > 0 ) { step_dir(1); step_bg--; }
	if( step_bg < 0 ) { step_dir(0); step_bg++; }
	schedule();
    }    
}


void task_run(void)
{
    while(1) {
        WDT_RESET();
	tcb_check();
	cl_parse();
	schedule();
    }    
}

/* init command line parser static memory array */
const cmd_t cl[] PROGMEM = {
    CMD(help),
    CMD(turn)    
};
const u8 max_cmd = ALEN(cl);




int main(void)
{
    // uitfam OS init start
    ser_init();
    timer2_init();
    adc_init();
    twi_init(1, TWI_ID );
    cl_init();
    // uitfam OS init complete 

    // APP init start
    step_init();
    task_create(task_run);
    task_create(task_stepper);    
    // APP init complete

    // uitfam OS activation
    sei();
    writeln("# " PROG_NAME );
    LOG("DEBUG LOG ENABLED");
    WDT_ENABLE();
    run(); /* never returns, defined inside scheduler */
}
