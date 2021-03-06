/**
*  @file cmdln.c
*
*/

/* command line parser impl. ------------- */
#include "cmdln.h"
#include "xitoa.h"
#include <avr/interrupt.h>

struct line_buf_st LN;
char (*cmd_buf_empty)(void);
u8 (*cmd_buf_get)(void);


static uint8_t cmd_find(char *buf);
static u8 get_line(void);
static inline cmd_fn_t *get_func(uint8_t a);


/**
*  @brief returns pointer to command function
*  @param[in] a index of function in array cl[]
*  @return pointer to requested function
*/
static inline cmd_fn_t *get_func(uint8_t a)
{
  return (cmd_fn_t*) pgm_read_word(& (cl[a ].fn)  );
}

/**
 *  @brief suche nach string *buf in tabelle cl
 *
 *  @return 1..n Nummer des gefundenen Kommandos,
 *  @return 0 falls nicht erkannt.
 */
static uint8_t cmd_find(char* buf)
{
  uint8_t i, u, ch;
  i = max_cmd;

LOOP:
  i--;
  for (u = 0; u < sizeof(cl[i].name); u++)
  {
    ch = pgm_read_byte(cl[i].name+u);
    if (ch == 0) break;
    if (ch != buf[u]) goto not_found;
  }
  return i + 1;

not_found: if (i) goto LOOP;
  return 0;
}

/**
 *  @brief  die funktion get_line holt die daten aus rbuf ab die daten sind mit 0 beendet.
 *
 *  @return 1 zeile gelesen
 *  @return 0 keine daten
 *
 *  die daten sind mit 0 beendet. chr(13) wird ignoriert.
 *  chr(10) wird nicht gespeichert und beendet das einlesen;
 *  zu lange zeilen werden abgeschnitten
 */
static u8 get_line(void)
{
  char ch;

  if (cmd_buf_empty()) return 0;

  /** @todo: bessere Q implementieren */

  cli();
  ch = cmd_buf_get();
  sei();

  if (ch == '\r') return 0;

  if (LN.len >= sizeof LN.buf) return 1;

  if (ch == '\n')
  {
    if (LN.len == 0) return 0;
    LN.buf[LN.len++] = 0;
    return 1;
  }

  if (LN.len < sizeof(LN.buf) - 1)
    LN.buf[LN.len++] = ch;

  return 0;
}

/**
* @brief capture the command received by MCU Serial input; if it exists in array cl[] (see cmd.c),
*        runs the command, otherwise outputs an error message
*
*/
void exec_cmd_line(void)
{
  u8 cmd_num;
  cmd_fn_t* func;

  if( get_line() )
  {
      cmd_num = cmd_find(LN.buf);
      if (cmd_num) {
              func = get_func(cmd_num - 1);
              func();
      } else {
          xputs(PSTR("ERR:"));
          u8 i;
          for (i = 0; i < LN.len; i++) xprintf(PSTR("%02X "), LN.buf[i]);
          xputc(10);
      }

      LN.len = 0; // free buffer, get ready for next line
  }
}

void cmd_help(void)
{
    u8 i;
    writeln("# help");


    for(i=0;i<max_cmd;i++) {
        const char *s = (const char *)pgm_read_word( &(cl[i].help) );
        if(! s) continue;
        write("# "); xputs(s); xputc(10);
    } 
}


u8 cl_num8(u8 *p, u8 *num)
{
    char *s = LN.buf+*p;
    long val;
    if( xatoi(&s,&val) ) {
	*num=val;
	*p = s - LN.buf;
	return 0;
    }
    return 1;
}
u8 cl_num16(u8 *p, u16 *num)
{
    char *s = LN.buf+*p;
    long val;
    if( xatoi(&s,&val) ) {
	*num=val;
	*p = s - LN.buf;
	return 0;
    }
    return 1;
}
