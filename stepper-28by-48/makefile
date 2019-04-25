# Hey Emacs, this is a -*- makefile -*-
include ../rules.mk
VPATH=../lib
CFLAGS+=-I../lib
MCU=atmega328p
FCPU=16000000UL
# AVRDUDE_PORT=/dev/ttyACM0
# AVRDUDE_PROGRAMMER = arduino
AVRDUDE_PORT=/dev/ttyUSB0
# AVRDUDE_PROGRAMMER = stk500v2
AVRDUDE_PROGRAMMER = arduino 
SRCS=timer2.c twi1.c cmdln.c tcb.c schedule.c 
SRCS+=adc.c 
ASM=xitoa.S
OBJS := $(SRCS:.c=.o)


master.bin: master.c $(OBJS) $(ASM)
