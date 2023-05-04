#include "states.h"
#include <EEPROM.h>

#define EEPROM_SIZE 21

extern byte networkID[4];
extern byte AESKey[16];

int func_boot() {
  if (EEPROM.begin(EEPROM_SIZE) == 0) {
    Serial.println("Fatal Error: Failed to initiate EEPROM.");
    return STATE_SLEEP;
  }
  if (EEPROM.readByte(0) == 0x67) {
    if (EEPROM.readBytes(1, AESKey, 16) == 16 && EEPROM.readBytes(17, networkID, 4) == 4) {
      Serial.println();
    }
    else {
      Serial.println("Fatal Error: Device EEPROM corrupted.");
      return STATE_SLEEP;
    }
  }
  else
  {
    
    Serial.println("Error: No configuratoin found.");
    return STATE_SLEEP;
  }
  EEPROM.end();
  return STATE_BOOT;
}
