#include "states.h"
#include <EEPROM.h>

#define EEPROM_SIZE 21

extern byte networkID[4];
extern byte AESKey[16];

int func_boot() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.readByte(0) == 0x67) {
    
  }
  else
  {
    return STATE_SLEEP;
  }
  return STATE_BOOT;
}
