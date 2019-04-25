# Hey Emacs, this is a -*- makefile -*-
#
VPATH=../lib
CFLAGS+=-I../lib
DEV=/dev/ttyUSB0
FCPU = 20000000UL
MCU = atmega644p
LOG  ?= 0
# AVRDUDE_PROGRAMMER = avrisp2 -P /dev/ttyAVRISP2
# AVRDUDE_PROGRAMMER = arduino -P /dev/ttyACM1
AVRDUDE_PROGRAMMER = stk500v2
TRACELEVEL=0
DEPDIR = .d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

CP=cp
MKDIR=mkdir
ASRC= xitoa.S
EXTRADEFINES += -I./ -DUSE_CONFIG_H
OPT = s
CC = avr-gcc
LD = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
HEXSIZE = avr-size --target=ihex $(TARGET).hex
NM = avr-nm
AVRDUDE = avrdude
LDFLAGS=-mmcu=$(MCU)
CFLAGS += $(DEPFLAGS)
CFLAGS += -mmcu=$(MCU)
CFLAGS += -DF_CPU=$(FCPU) $(EXTRADEFINES)  -O$(OPT)
CFLAGS += -std=gnu99
CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += -Wall -Wstrict-prototypes
CFLAGS += -Wa,-adhlns=$(@).lst
CFLAGS += -ffunction-sections
ASFLAGS += -mmcu=$(MCU) -x assembler-with-cpp -g
AVRDUDE_FLAGS = -p $(MCU) -c $(AVRDUDE_PROGRAMMER) -P $(AVRDUDE_PORT) -V
REVISION_HG = $(shell hg identify --id --branch --tags | sed -e "s/ /:/g")
CFLAGS += -DHGREVISION=\"$(REVISION)\"
CFLAGS+=-Xlinker -Map=output.map


all: master.hex

%.o: %.c
	$(CC) -c $(CFLAGS) -DLOG_ENABLE=$(LOG) -o $@ $^ $(LOADLIBES) $(LDFLAGS)
	$(POSTCOMPILE)

%.bin: %.c
	$(CC) $(CFLAGS) -DLOG_ENABLE=$(LOG) -o $@ $^ $(LOADLIBES) $(LDFLAGS)
	$(POSTCOMPILE)

%.lst: %.c
	$(CC) $(CFLAGS) -c -g -Wa,-a,-ad $^ >$@

%.hex : %.bin
	avr-size $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	-${RM} *.bin *.bin_log *.bin_debug *.o *.hex .dep/* *.lst *.map *.ihex rev.txt *.out *~

isp: master.hex
	-killall -s USR1 xcom
	sleep .1
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:master.hex:i
	sleep .1
	-killall -s USR2 xcom

fuses:
	-killall -s USR1 xcom
	sleep .1
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(FUSES)
	sleep .1
	-killall -s USR2 xcom

$(DEPDIR)/%.d: ;
