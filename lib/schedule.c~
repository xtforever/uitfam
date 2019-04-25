#include "schedule.h"
#include "cpu.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/* memory layout
MEM = &TASK[TASK_CURRENT]
MEM [ STACK_SIZE - 1  ]      = function_low
MEM [ STACK_SIZE - 2  ]      = function_high
MEM [ STACK_SIZE - 3  ]      = R31
...
MEM [ STACK_SIZE - 34 ]      = R0

SP+34   HIGH
SP+33   LOW
SP+32   R31
...
SP+1    R0
SP
*/


struct task_st {
    u8  stack[STACK_SIZE];
    u16 sp;
};

struct task_st TASK[NUM_TASKS];
uint8_t TASK_CURRENT = 0;
uint8_t TASK_MAX     = 0;

#define PUSH(r) asm volatile("push r"#r);
#define POP(r) asm volatile("pop r"#r);
static inline u16 GET_SP(void) { return SP; }
static inline void SET_SP(u16 sp) {  cli(); SP=sp; sei(); }

void schedule(void) __attribute__ ( ( naked ) );
void schedule(void)
{
    PUSH(31);PUSH(30);PUSH(29);PUSH(28);PUSH(27);PUSH(26);PUSH(25);PUSH(24);
    PUSH(23);PUSH(22);PUSH(21);PUSH(20);PUSH(19);PUSH(18);PUSH(17);PUSH(16);
    PUSH(15);PUSH(14);PUSH(13);PUSH(12);PUSH(11);PUSH(10);PUSH( 9);PUSH( 8);
    PUSH( 7);PUSH( 6);PUSH( 5);PUSH( 4);PUSH( 3);PUSH( 2);PUSH( 1);PUSH( 0);
    TASK[TASK_CURRENT].sp = GET_SP();
    WDT_RESET();
    TASK_CURRENT++;
    if( TASK_CURRENT >= TASK_MAX ) TASK_CURRENT=0;
    SET_SP( TASK[TASK_CURRENT].sp );
    POP( 0);POP( 1);POP( 2);POP( 3);POP( 4);POP( 5);POP( 6);POP( 7);
    POP( 8);POP( 9);POP(10);POP(11);POP(12);POP(13);POP(14);POP(15);
    POP(16);POP(17);POP(18);POP(19);POP(20);POP(21);POP(22);POP(23);
    POP(24);POP(25);POP(26);POP(27);POP(28);POP(29);POP(30);POP(31);
    asm volatile("ret");
}

void run(void) __attribute__ ( ( naked ) );
void run(void)
{
    TASK[TASK_CURRENT=0].sp+=32;  /* nothing to POP here */
    SET_SP( TASK[TASK_CURRENT=0].sp );
    asm volatile("ret"); /* return to function of first task */
}

void task_create(void *f)
{
    struct task_st *t = TASK + TASK_MAX++;
    t->sp =(u16) (t->stack + STACK_SIZE - 3 - 32);
    t->stack[STACK_SIZE-2] = _HI8_(f);
    t->stack[STACK_SIZE-1] = _LO8_(f);
}
