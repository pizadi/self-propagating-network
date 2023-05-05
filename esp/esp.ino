#include <mbedtls/aes.h>
#include "states.h"
#include "globals.h"

int STATE;

void setup() {
  Serial.begin(115200);
  Serial.onReceive(func_config, true);
  STATE = 1;
}

void loop() {
  while (true) {
    switch (STATE) {
      case STATE_SLEEP:
      STATE = func_sleep();
      break;
      case STATE_BOOT:
      STATE = func_boot();
      break;
      default:
      Serial.println("Fatal Error in the state machine. Please reboot.");
      STATE = func_sleep();
      break;
    }
  }
}
