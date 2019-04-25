#ifndef SERX_H
#define SERX_H

#include "config.h"
#include "cpu.h"
#include "xbuf.h"


#define BAUD_TO_REG(baud) ((F_CPU / 16 / (baud)) -1)
#define TX_BUSY(num) (!(UCSR ## num ## A & _BV(UDRE ## num )))


#if defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) || defined  (__AVR_ATmega88__)
#  define USART0_RX_vect USART_RX_vect
#  define USART0_TX_vect USART_TX_vect
#endif

#define GEN_SERX_HDR_SER(pref)				\
  void pref ## _check        ( void );			\
  void pref ## _put          ( char ch );		\
  void pref ## _init         ( u16 reg );		\
  u8   pref ## _get          ( void );			\
  char pref ## _out_getc     (void);			\
  char pref ## _in_getc      (void);			\
  uint8_t pref ## _out_get   (void);			\
  uint8_t pref ## _in_get    (void);

#define GEN_SERX_HDR_BUF(pref)				\
  char pref ## _is_empty ( void );			\
  char pref ## _is_full  (void);			\
  void pref ## _put      ( char b );   \
  void pref ## _init     ( void );     \
  int  pref ## _queue_len(void);

#define GEN_SERX_HDR(pref)			\
  GEN_SERX_HDR_SER(pref)			\
  GEN_SERX_HDR_BUF( pref ## _in )		\
  GEN_SERX_HDR_BUF( pref ## _out ) \
  extern volatile uint8_t TXIRQ ## num ## RUNNING;


#define GEN_SERX(pref, num,len)                         \
    GEN_XBUF(pref ## _out ,len)                         \
    GEN_XBUF(pref ## _in, len)                          \
    volatile uint8_t TXIRQ ## num ## RUNNING = 0; \
                                                         \
ISR( USART ## num ## _TX_vect )                      \
{                                                     \
  if( pref ## _out_is_empty() )                       \
    TXIRQ ## num ## RUNNING = 0;                      \
  else {                                           \
    UDR ## num  = pref ## _out_get();                \
    TXIRQ ## num ## RUNNING = 1;                   \
  }                                                  \
}                                                    \
                                                     \
ISR(USART ## num ## _RX_vect)                   \
{						\
  pref ## _in_put( UDR ## num  );		\
}						\
						\
void pref ## _check(void)			\
{						\
  if(! pref ## _out_is_empty() ) {		\
    if(!TXIRQ ## num ## RUNNING)                \
      if( ! TX_BUSY(num)) {                     \
            UDR ## num = pref ## _out_get();    \
            TXIRQ ## num ## RUNNING = 1;        \
      }                                         \
  }						\
}						\
uint8_t pref ## _get(void)                      \
{	                                        \
  cli();                                        \
  uint8_t ch = pref ## _in_get();               \
  sei(); return ch;                             \
}						\
void pref ## _put(char ch)			\
{	                                        \
  uint16_t i= (F_CPU/BAUD) * 4;                 \
  cli();                                        \
  if( pref ## _out_is_full() ) {                \
    while( TX_BUSY(num) && i--) WDT_RESET();    \
    UDR ## num  = pref ## _out_getc();          \
    TXIRQ ## num ## RUNNING = 1;              \
    pref ## _out_put(ch);                     \
    sei();                                  \
    return;                                   \
  }                                             \
  pref ## _out_put(ch);                         \
  if( !TXIRQ ## num ## RUNNING )                \
  {                                             \
    while( TX_BUSY(num) && i--) WDT_RESET();    \
    UDR ## num  = pref ## _out_getc();          \
    TXIRQ ## num ## RUNNING = 1;                \
  }                                             \
  sei();                                        \
}						\
						\
void pref ## _init( u16 reg )			\
{						\
  TXIRQ ## num ## RUNNING = 0;                  \
  UBRR ## num  = reg;				\
  UCSR ## num ## B =				\
    _BV(TXEN ## num  ) |			\
    _BV(RXEN ## num  ) |			\
    _BV(RXCIE ## num  ) |			\
    _BV(TXCIE ## num  );			\
						\
  UCSR ## num ## A = 0;				\
  UCSR ## num ## C = (3 << UCSZ00);		\
}

#endif
