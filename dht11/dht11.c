#include "config.h"
#include <util/delay.h>

//	Test DHT11 (Temperatur und Luftfeuchtigkeitssensor)
// 
//		 _________
//		|  -+-+-  |
//		| +-+-+-+ |
//		| +-+-+-+ |
//		|  -+-+-  |
//		| +-+-+-+ |
//		|_________|
//		  | | | |
//		  1 2 3 4
//
//	1. VCC (3 to 5V power)
//	2. Data out (Pullup 4,7k)
//	3. Not connected
//	4. Ground
//
#define DHT11_out_low()		SBI_DDR(DHT11)
#define DHT11_in()		CBI_DDR(DHT11)
#define DHT11_is_hi()		GET_PIN(DHT11)
#define DHT11_is_low()		!DHT11_is_hi()

#define WHILE_HI(x)				\
    do { u8 timeout=255;			\
	while(DHT11_is_hi()) {			\
	_delay_us(2);				\
	if (!timeout--) { return (x); }		\
    } } while(0)
#define WHILE_LO(x)				\
    do { u8 timeout=255;			\
	while(DHT11_is_low()) {			\
	_delay_us(2);				\
	if (!timeout--) { return (x); }		\
    } } while(0)

static int dht11_read(void)
{
    u8 dht11byte=0;
    for( uint8_t j=0; j<8; j++) {
	dht11byte <<= 1;
	WHILE_LO(-5);
	// Start to transmit 1-Bit (50 us)
	_delay_us(30);
	// Hi > 30us (70 us) -> Bit=1
	// Hi <  30us (26-28 us) -> Bit=0	
	if (DHT11_is_hi()) {	
	    WHILE_HI(6);
	    dht11byte |= 1;
	}
    }
    return dht11byte;
}

uint8_t dht11(u8 *feuchte, u8 *temperatur) 
{
    u8 sum=0;
    if (DHT11_is_low()) {return 1;} // Bus not free
    DHT11_out_low();		// MCU start signal (>=18MS) 
    _delay_ms(20);		/*  */
    DHT11_in();
    _delay_us(15);
    WHILE_HI(2); // Wait for DHT's response (20-40us)
    WHILE_LO(3); // Response signal (80us)
    WHILE_HI(4); // Preparation for sending data (80us)

    int ret=0;

    ret = dht11_read();
    if( ret < 0 ) return 5;
    sum += ret;
    *feuchte = ret;

    ret = dht11_read();
    if( ret < 0 ) return 5;
    sum += ret;

    ret = dht11_read();
    if( ret < 0 ) return 5;
    sum += ret;
    *temperatur = ret;

    ret = dht11_read();
    sum += ret;

    ret = dht11_read();
    if(sum != ret) return 7;	// Checksum error
    return 0;
}


