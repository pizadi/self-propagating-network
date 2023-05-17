#include <mbedtls/aes.h>
#include <esp_random.h>
#include "states.h"
#include "globals.h"

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  Serial.onReceive(func_config, true);
  Serial2.onReceive(packetReceive, false);
  broadcastTimer = timerBegin(0, TIMER_PRESCALER, true);
  globalTimer = timerBegin(1, TIMER_PRESCALER, true);
  timerAttachInterrupt(broadcastTimer, &broadcastTimerInterrupt, true);
  tempID = esp_random();
  STATE = STATE_BOOT;
}

void loop() {
//  Serial.println(timerRead(broadcastTimer), DEC);
  switch (STATE) {
    case STATE_SLEEP:
    STATE = func_sleep();
    break;
    case STATE_BOOT:
    STATE = func_boot();
    break;
    case STATE_SEARCH:
    STATE = func_search();
    break;
    case STATE_STEADY:
    STATE = func_steady();
    break;
    default:
    Serial.println("Fatal Error in the state machine. Trying to reboot...");
    ESP.restart();
    STATE = func_sleep();
    break;
  }
}
