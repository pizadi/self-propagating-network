#include <Preferences.h>

#include "states.h"
#include "globals.h"


int func_boot() {
  preferences.begin("config", true);
  if (preferences.isKey("KEY") && preferences.isKey("ID")) {
    if (preferences.getBytes("KEY", secret, 30) == 30 && preferences.getBytes("ID", networkID, 4) == 4) {
      Serial.println("Info: Configuration loaded successfully.");
    }
    else {
      Serial.println("Error: Configuratoin corrupted.");
      return STATE_SLEEP;
    }
  }
  else {
    Serial.println("Error: No configuratoin found.");
    return STATE_SLEEP;
  }
  return STATE_SEARCH;
}
