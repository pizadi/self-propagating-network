#ifndef GLOBALS_H
#define GLOBALS_H
#include <mbedtls/aes.h>
#include <Preferences.h>
#include "esp_system.h"

// DEFINES

#define TIMEOUT 100
#define MAX_TIMEOUTS 5
#define MAX_WAIT 800
#define MIN_WAIT 200
#define MAX_BC_FIFO 24
#define MAX_INPUT_FIFO 512

#define TIMER_PRESCALER 80
#define TIMER_MULTIPLIER 1000

// STRUCTS

typedef struct {
  uint8_t len;
  byte * message;
} packet;

// VARIABLES

extern uint8_t STATE; // The global state for the state machine
extern bool trafficFlag; // A flag which indicates whether the channel was empty during the wait period
extern int broadcastFIFO_head; // Index for the transmission FIFO's head
extern int broadcastFIFO_len; // Length of the transmission FIFO
extern int inputFIFO_head; // Index for the transmission FIFO's head
extern int inputFIFO_len; // Length of the transmission FIFO
extern bool broadcastTimerIsSet; // Indicates whether the broadcast timer was already set
extern hw_timer_t * broadcastTimer; // General Transmission timer
extern hw_timer_t * globalTimer; // Global timer
extern uint16_t tempID;

// REGISTERS

extern byte networkID[4];
extern byte deviceID[4];
extern byte parentID[4];
extern byte secret[30];

extern byte inputFIFO[512];
extern packet broadcastFIFO[MAX_BC_FIFO]; // The global broadcast FIFO

extern byte payloadBuffer[240];
extern byte encBuffer[240];
extern byte ivBuffer[16];
extern char password[236];
extern char debugInterrupt[192]; 

// OBJECTS

extern mbedtls_aes_context aesctx;
extern Preferences preferences;

// FUNCTIONS

void schedule_broadcast(byte * packet, uint8_t len);
void setBroadcastTimer();
void broadcastPacket();
byte * parseHead(uint8_t flags);
bool isChild(byte * id);
void overrideParent(byte * current, byte * candidate);
void setParent();
void decrypt(byte * IV, byte * src, uint8_t len, byte * out);

// INTERRUPTS

void IRAM_ATTR broadcastTimerInterrupt();
void IRAM_ATTR packetReceive();
#endif
