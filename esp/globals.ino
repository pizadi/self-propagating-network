#include <mbedtls/aes.h>
#include <Preferences.h>

byte messageBuffer[16];
byte networkID[4];
byte deviceID[4];
byte AESKey[16];
byte payloadBuffer[240];
byte encBuffer[240];
byte ivBuffer[16];

char password[236] = "espressif";

mbedtls_aes_context aesctx;

Preferences preferences;
