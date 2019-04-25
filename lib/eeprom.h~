#ifndef  EEPROM_H
#define  EEPROM_H

enum EEP_ACTION {
    EEP_READ =   0,
    EEP_WRITE =  1
};

static inline void eeprom(enum EEP_ACTION write, uint16_t addr, void* buf, uint16_t size)
{
    WDT_DISABLE();

  if (write)
    eeprom_write_block(buf, (void*) addr, size);
  else
    eeprom_read_block(buf, (void*) addr, size);

  WDT_ENABLE();
}


#endif
