#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <avr/io.h>

// Lo-Byte First on target CPU (AVR)
#define TARGET_HI 1
#define TARGET_LO 0

// Lo-Byte First on host CPU
#define HOST_HI 1b
#define HOST_LO 0

typedef int16_t s16;
typedef uint16_t u16;
typedef uint8_t uchar;
typedef uint8_t u8;
typedef uint32_t u32;
typedef int32_t s32;

typedef union
{
  s16 v;
  u8 b[2];
} sn16;

typedef union
{
  u16 v;
  u8 b[2];
} un16;

typedef union
{
  uint32_t v;
  u8 b[4];
} un32;

/* function prototypes */
inline void sbi_port(uint8_t S, uint8_t x);
inline void sbi_ddr(uint8_t S, uint8_t x);
inline void cbi_port(uint8_t S, uint8_t x);
inline void cbi_ddr(uint8_t S, uint8_t x);
inline uint8_t get_pin(uint8_t S, uint8_t x);
inline uint8_t get_port(uint8_t S, uint8_t x);
inline void xbi_port(uint8_t S, uint8_t x);
inline void cfg_in_x(uint8_t a);
inline void cfg_hi_x(uint8_t a);
inline void cfg_lo_x(uint8_t a);
inline void cfg_out_x(uint8_t a);
inline void cfg_in_pullup_x(uint8_t a);
inline u8 id8(u8 x);

#define ULOW( x ) (x).b[TARGET_LO]
#define UHIGH( x ) (x).b[TARGET_HI]

#define _LO8_( x ) ULOW( *((un16*)&(x)) )
#define _HI8_( x ) UHIGH( *((un16*)&(x)) )

#define HEX(x) BIN2HEX(id8(x))
#define BIN2HEX(x) (x) > 9 ? (x) + 55 : (x) + '0'
#define XTOI(a) ((a) > '9' ? ((a)-55) : ((a)-'0'))
#define ALEN(x) (sizeof(x)/sizeof(*(x)))

#define xEXPAND(a) #a
#define STR(a) xEXPAND(a)

#define DDR_COMPOSE(a) DDR ## a
#define V_DDR(a) DDR_COMPOSE(a)

#define PORT_COMPOSE(a) PORT ## a
#define V_PORT(a) PORT_COMPOSE(a)

#define DDRPORT_SET(a,b) do { V_DDR(a) |= _BV(b); V_PORT(a) |= _BV(b); } while(0)
#define SDDRCPORT(a,b) do { V_DDR(a) |= _BV(b); V_PORT(a) &= ~_BV(b); } while(0)

inline u8 id8(u8 x)
{
  return x;
}

inline void sbi_port(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef PORTA
  else if( S == 1 ) {
    PORTA |= x;
  }
#endif

#if defined(PORTB)
  else if( S == 2 ) {
    PORTB |= x;
  }
#endif

#if defined(PORTC)
  else if( S == 3 ) {
    PORTC |= x;
  }
#endif

#if defined(PORTD)
  else if( S == 4 ) {
    PORTD |= x;
  }
#endif
}

inline void sbi_ddr(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef DDRA
  else if( S == 1 ) {
    DDRA |= x;
  }
#endif
#if defined(DDRB)
  else if( S == 2 ) {
    DDRB |= x;
  }
#endif
#if defined(DDRC)
  else if( S == 3 ) {
    DDRC |= x;
  }
#endif
#if defined(DDRD)
  else if( S == 4 ) {
    DDRD |= x;
  }
#endif
}

inline void xbi_ddr(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef DDRA
  else if( S == 1 ) {
    DDRA ^= x;
  }
#endif
#if defined(DDRB)
  else if( S == 2 ) {
    DDRB ^= x;
  }
#endif
#if defined(DDRC)
  else if( S == 3 ) {
    DDRC ^= x;
  }
#endif
#if defined(DDRD)
  else if( S == 4 ) {
    DDRD ^= x;
  }
#endif
}

inline void cbi_port(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef PORTA
  else if( S == 1 ) {
    PORTA &= ~x;
  }
#endif
#if defined(PORTB)
  else if( S == 2 ) {
    PORTB &= ~x;
  }
#endif
#if defined(PORTC)
  else if( S == 3 ) {
    PORTC &= ~x;
  }
#endif
#if defined(PORTD)
  else if( S == 4 ) {
    PORTD &= ~x;
  }
#endif
}

inline void cbi_ddr(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef DDRA
  else if( S == 1 ) {
    DDRA &= ~x;
  }
#endif
#if defined(DDRB)
  else if( S == 2 ) {
    DDRB &= ~x;
  }
#endif
#if defined(DDRC)
  else if( S == 3 ) {
    DDRC &= ~x;
  }
#endif
#if defined(DDRD)
  else if( S == 4 ) {
    DDRD &= ~x;
  }
#endif
}


inline uint8_t get_pin(uint8_t S, uint8_t x)
{
  if (x == 0) return 0;
#ifdef PINA
  else if( S == 1 ) {
    return PINA & x;
  }
#endif
#if defined(PINB)
  else if( S == 2 ) {
    return PINB & x;
  }
#endif
#if defined(PINC)
  else if( S == 3 ) {
    return PINC & x;
  }
#endif
#if defined(PIND)
  else if( S == 4 ) {
    return PIND & x;
  }
#endif

  return 0;
}


inline uint8_t get_port(uint8_t S, uint8_t x)
{
  if (x == 0) return 0;
#ifdef PINA
  else if( S == 1 ) {
    return PORTA & x;
  }
#endif
#if defined(PINB)
  else if( S == 2 ) {
    return PORTB & x;
  }
#endif
#if defined(PINC)
  else if( S == 3 ) {
    return PORTC & x;
  }
#endif
#if defined(PIND)
  else if( S == 4 ) {
    return PORTD & x;
  }
#endif

  return 0;
}


inline void xbi_port(uint8_t S, uint8_t x)
{
  if (x == 0)
  {
  }
#ifdef PORTA
  else if( S == 1 ) {
    PORTA ^= x;
  }
#endif
#if defined(PORTB)
  else if( S == 2 ) {
    PORTB ^= x;
  }
#endif
#if defined(PORTC)
  else if( S == 3 ) {
    PORTC ^= x;
  }
#endif
#if defined(PORTD)
  else if( S == 4 ) {
    PORTD ^= x;
  }
#endif
}

inline void cfg_in_x(uint8_t a)
{
  cbi_ddr((a / 16) - 9, _BV(a&0x07));
  cbi_port((a / 16) - 9, _BV(a&0x07));
}

inline void cfg_hi_x(uint8_t a)
{
  sbi_ddr((a / 16) - 9, _BV(a&0x07));
  sbi_port((a / 16) - 9, _BV(a&0x07));
}

inline void cfg_lo_x(uint8_t a)
{
  sbi_ddr((a / 16) - 9, _BV(a&0x07));
  cbi_port((a / 16) - 9, _BV(a&0x07));
}

inline void cfg_out_x(uint8_t a)
{
  sbi_ddr((a / 16) - 9, _BV(a&0x07));
}

inline void cfg_in_pullup_x(uint8_t a)
{
  cbi_ddr((a / 16) - 9, _BV(a&0x07));
  sbi_port((a / 16) - 9, _BV(a&0x07));
}


#define MAKE_HEX2(a) 0x ## a
#define MAKE_HEX(a) MAKE_HEX2(a)

#define cfg_in(a) cfg_in_x( MAKE_HEX(a) )
#define cfg_in_pullup(a) cfg_in_pullup_x( MAKE_HEX(a) )
#define cfg_hi(a) cfg_hi_x( MAKE_HEX(a) )
#define cfg_lo(a) cfg_lo_x( MAKE_HEX(a) )
#define cfg_out(a) cfg_out_x( MAKE_HEX(a) )

#define CBI_DDRx( a ) cbi_ddr( (a/16)-9, _BV(a&0x07))
#define SBI_DDRx( a ) sbi_ddr( (a/16)-9, _BV(a&0x07))
#define XBI_DDRx( a ) xbi_ddr( (a/16)-9, _BV(a&0x07))

#define CBI_PORTx( a ) cbi_port( (a/16)-9, _BV(a&0x07))
#define SBI_PORTx( a ) sbi_port( (a/16)-9, _BV(a&0x07))
#define XBI_PORTx( a ) xbi_port( (a/16)-9, _BV(a&0x07))

#define GET_PINx( a ) get_pin( (a/16)-9, _BV(a&0x07))
#define GET_PORTx( a ) get_port( (a/16)-9, _BV(a&0x07))

#define CBI_DDR( a ) CBI_DDRx( MAKE_HEX(a) )
#define SBI_DDR( a ) SBI_DDRx( MAKE_HEX(a) )
#define XBI_DDR( a ) XBI_DDRx( MAKE_HEX(a) )
#define CBI_PORT( a ) CBI_PORTx( MAKE_HEX(a) )
#define SBI_PORT( a ) SBI_PORTx( MAKE_HEX(a) )
#define XBI_PORT( a ) XBI_PORTx( MAKE_HEX(a) )
#define GET_PIN( a ) GET_PINx( MAKE_HEX(a) )
#define GET_PORT( a ) GET_PORTx( MAKE_HEX(a) )

#if LOG_ENABLE > 1
#define LOG2(x,a...) xprintf( PSTR(x"\n"), ## a )
#else
#define LOG2(x,a...) do {} while(0)
#endif

#if LOG_ENABLE == 1
#define LOG(x,a...) xprintf( PSTR(x"\n"), ## a )
#else
#define LOG(x,a...) do {} while(0)
#endif

/* trace macro */
#define TR(x,a...) while( LOG_ENABLE ) { xprintf( PSTR( x "\n"), ## a ); break; }

/* parameter syntax error */
#define ERR(x,a...) xprintf( PSTR("ERR " x "\n" ), ## a )

/* program logik error  */
#define ERI(x,a...) xprintf( PSTR( "ERI %s:" x "\n" ),__func__,  ## a )

#define writeln(x,a...) xprintf( PSTR(x"\n"), ## a )
#define write(x,a...) xprintf( PSTR(x), ## a )

#define BAUD_TO_REG(baud) ((F_CPU / 16 / (baud)) -1)
#define BAUD_TO_REG_DOUBLE_MCK(baud) ((F_CPU / 16 / (baud/2)) -1)

#endif
