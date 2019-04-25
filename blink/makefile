# Hey Emacs, this is a -*- makefile -*-
include ../rules.mk

# arduino pro mini via ttl-usb rts=dtr
# MCU=atmega328p
# FCPU=16000000UL
# AVRDUDE_PORT=/dev/ttyUSB0 
# AVRDUDE_PROGRAMMER = arduino -b57600

# arduino uno r3
MCU=atmega328p
FCPU=16000000UL
AVRDUDE_PORT=/dev/ttyUSB0 
AVRDUDE_PROGRAMMER = arduino -b115200



SRCS=timer2.c cmdln.c tcb.c adc.c
ASM=xitoa.S
OBJS := $(SRCS:.c=.o)

master.bin: master.c $(OBJS) $(ASM)

## include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

