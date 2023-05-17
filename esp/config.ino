#include <Preferences.h>
#include <string.h>

#include "states.h"
#include "globals.h"



void func_config() {
  char passBuffer[236];
  uint8_t i = 0;
  while (Serial.available() && i < 256) {
    passBuffer[i] = Serial.read();
    if (passBuffer[i] == '\0') break;
    i++;
  }
  if (strcmp(password, passBuffer) == 0) {
    if (Serial.available() != 34) {
      while (Serial.available()) Serial.read();
      Serial.print("WRPK");
      Serial.flush();
      Serial.end();
      return;
    }
    
    for (int i = 0; i < 30; i++) secret[i] = Serial.read();
    for (int i = 0; i < 4; i++) networkID[i] = Serial.read();
    
    preferences.begin("config", false);
    preferences.clear();
    preferences.putBytes("KEY", (void *) secret, 30);
    preferences.putBytes("ID", (void *) networkID, 4);
    preferences.end();
    Serial.print("DONE");
    Serial.flush();
    ESP.restart();
  }
  else {
    Serial.print("WRPW");
    Serial.flush();
  }
}
