#include <mbedtls/aes.h>
#include <esp_random.h>
#include <esp32/rom/crc.h>
#include "states.h"

#define TIMEOUT
#define MAX_TIMEOUTS

byte messageBuffer[16];
byte networkID[4];
byte deviceID[4];
byte AESKey[16];
byte payloadBuffer[240];
byte encBuffer[240];
byte ivBuffer[16];

char * password = "espressif";

mbedtls_aes_context aesctx;

int STATE;

void setup() {
  Serial.begin(115200);
  Serial.onReceive(func_config, true);
  STATE = 0;
}

void loop() {
  while (true) {
    switch (STATE) {
      case STATE_BOOT:
      STATE = func_boot();
      break;
      default:

      break;
    }
  }
}

void onSerialRx() {
  while (Serial.available()) Serial.read();
  Serial.println("INTERRUPTED!");
}
