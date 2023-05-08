#include <mbedtls/aes.h>
#include <Preferences.h>
#include <esp_random.h>

// LOCAL INCLUDES

#include "globals.h"

// VARIABLES

uint8_t STATE; // The global state for the state machine
bool trafficFlag = false; // A flag which indicates whether the channel was empty during the wait period
//packet * broadcastFIFO = new packet[MAX_FIFO];
int broadcastFIFO_head = 0;
int broadcastFIFO_len = 0;
int inputFIFO_head = 0;
int inputFIFO_len = 0;
bool broadcastTimerIsSet = false;
hw_timer_t * broadcastTimer = NULL;
uint32_t parent = 0;

// REGISTERS

byte messageBuffer[16];
byte networkID[4];
byte deviceID[4];
byte AESKey[16];

byte inputFIFO[512]; // Radio Input FIFO
packet broadcastFIFO[MAX_FIFO]; // The global broadcast FIFO

byte payloadBuffer[240];
byte encBuffer[240];
byte ivBuffer[16];
char password[236] = "espressif";
char debugInterrupt[192];

// OBJECTS

mbedtls_aes_context aesctx;
Preferences preferences;

// FUNCTIONS

void schedule_broadcast(byte * packet, uint8_t len) {
  int t = (broadcastFIFO_head + broadcastFIFO_len) % MAX_FIFO;
  if (broadcastFIFO_len >= MAX_FIFO) {
    delete[] broadcastFIFO[broadcastFIFO_head].message;
    broadcastFIFO_head = (broadcastFIFO_head + 1) % MAX_FIFO;
  }
  else {
    broadcastFIFO_len++;
  }
  broadcastFIFO[t].message = new byte[len];
  broadcastFIFO[t].len = len;
  for (int i = 0; i < len; i++) broadcastFIFO[t].message[i] = packet[i];
  setBroadcastTimer();
  packet = NULL;
}

void setBroadcastTimer() {
  if (broadcastTimerIsSet == false) {
    trafficFlag = false;
    uint64_t waitTime = esp_random() % (MAX_WAIT - MIN_WAIT) + MIN_WAIT;
    timerAlarmWrite(broadcastTimer, waitTime * TIMER_MULTIPLIER, false);
    timerWrite(broadcastTimer, 0);
    timerAlarmEnable(broadcastTimer);
    broadcastTimerIsSet = true;
  }
}

void broadcastPacket() {
  byte * packet = broadcastFIFO[broadcastFIFO_head].message;
  int len = broadcastFIFO[broadcastFIFO_head].len;
  for (int i = 0; i < len; i++) {
    while (Serial2.availableForWrite() == 0); // Risk of infinite loop in case of radio module malfunction, rewrite later
    Serial2.write(packet[i]);
  }
  Serial2.flush();
  Serial.println("Packet sent.");
  delete[] packet;
  broadcastFIFO_head = (broadcastFIFO_head + 1) % MAX_FIFO;
  broadcastFIFO_len -= 1;
  broadcastTimerIsSet = false;
  if (broadcastFIFO_len > 0) setBroadcastTimer();
}

// INTERRUPTS

void IRAM_ATTR broadcastTimerInterrupt() {
  broadcastTimerIsSet = false;
  if (trafficFlag){
    trafficFlag = false;
    uint64_t waitTime = esp_random() % (MAX_WAIT - MIN_WAIT) + MIN_WAIT;
    timerWrite(broadcastTimer, 0);
    timerAlarmWrite(broadcastTimer, waitTime * TIMER_MULTIPLIER, false);
    timerAlarmEnable(broadcastTimer);
  }
  else {
    broadcastPacket();
  }
}

void IRAM_ATTR packetReceive() {
  while (Serial2.available()) Serial2.read();
}
