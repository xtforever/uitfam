/**
*  @file cmdln.h
*/

#ifndef CMDLN2_H
#define CMDLN2_H
#include <avr/pgmspace.h>

#include "config.h"
#include "cpu.h"
#include <string.h>

/* command line parser ------------------- */
struct line_buf_st { u8 len; char buf[80]; };

typedef void cmd_fn_t(void);

typedef struct cmd_st {
    char name[6];
    cmd_fn_t *fn;
    const char *help;
} cmd_t;

/** struct to hold commands recieved by MCU Serial interface 
*	- defined in cmdln.c
*	- populated in exec_cmd_line()
*/
extern struct line_buf_st LN;

extern const cmd_t cl[] PROGMEM;	    /**< array of available commands */	
extern const u8 max_cmd;              /**< number of available commands */
extern char (*cmd_buf_empty) (void);
extern u8 (*cmd_buf_get) (void);

void exec_cmd_line(void);
static inline void cl_parse(void) { exec_cmd_line(); }
static inline void cl_init(void)  { LN.len=0;  }

u8 cl_num8(u8 *p, u8 *num);
u8 cl_num16(u8 *p, u16 *num);

#define CMD(x) { #x, cmd_ ## x, CMD_STR_ ## x }
#define HELP_MSG(X,Y) static const char CMD_STR_ ## X[] PROGMEM = Y
#define CMD_STR_help 0
void cmd_help(void);

#endif
