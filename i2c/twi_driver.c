#include "config.h"
#include "twi1.h"

u8 twi_device;

static void i2c_scanner_kill(void)
{
    TWHDL.fn[I2C_SCANNER] = 0;	/* disable scanner */
}

static void device_found(void)
{
    LOG("device found: %u", twi_device );
    writeln("+TWIDEV=%u", twi_device);
    i2c_scanner_kill();
}

static void device_not_found(void)
{
    LOG("scanning complete" );
    i2c_scanner_kill();
}

static u8 i2c_scanner_message(u8 state)
{
    switch( state ) {
    case TWI_READY:
	LOG("starting read: %u", twi_device);
	TWMEM[0] = id8( twi_device << 1);
	TWMEM[1] = 0;
	TWMEM[2] = 0xae;
	TWI.rw_start = 0;
	TWI.rw_max   = 3;
	TWI.read_sw   = 255;
	twi_start();
	// twi_read_from(0,twi_device,0,0 );
        return TWI_BUSY;
    case TWI_BUSY:
        if( (TWI.stat & TW_MT_RXC) ||
	    (TWI.stat & TW_MT_TXC) ) { 
	    device_found();
	} else {
	    LOG("ADDR %u, STAT: %u", twi_device, TWI.stat ); 
	    twi_device++;
	    if( twi_device > 126 )
		device_not_found();
	}
        return TWI_READY;
    }
    return TWI_READY;
}

void i2c_scanner_start(u8 addr)
{
    twi_device = addr;
    twhdl_register(I2C_SCANNER, i2c_scanner_message);
}
