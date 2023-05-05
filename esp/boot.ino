#include <EEPROM.h>

#include "states.h"
#include "globals.h"


int func_boot() {
  preferences.begin("config", true);
  if (preferences.isKey("KEY") && preferences.isKey("ID")) {
    if (preferences.getBytes("KEY", AESKey, 16) == 16 && preferences.getBytes("ID", networkID, 4) == 4) {
      Serial.println("Info: Configuration loaded successfully.");
    }
    else {
      Serial.println("Error: Configuratoin corrupted.");
    }
  }
  else {
    Serial.println("Error: No configuratoin found.");
    return STATE_SLEEP;
  }
  return STATE_SLEEP;
}
