#ifndef GLOBALS_H
#define GLOBALS_H

#include <mbedtls/aes.h>
#include <Preferences.h>

#define TIMEOUT 100
#define MAX_TIMEOUTS 5

extern byte messageBuffer[16];
extern byte networkID[4];
extern byte deviceID[4];
extern byte AESKey[16];
extern byte payloadBuffer[240];
extern byte encBuffer[240];
extern byte ivBuffer[16];

extern char password[236];

extern mbedtls_aes_context aesctx;

extern Preferences preferences;

#endif
