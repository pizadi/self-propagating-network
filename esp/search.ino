#include <Preferences.h>

#include "states.h"
#include "globals.h"


int func_search() {
  if (inputFIFO_len > 0) {
    byte header[16];
    parseHead(header, HEADER_ADP);
    if (header[0]) {
      // TODO
      // set parent, send ACK packet, and go to stable state
      parent = header[8] + ((uint32_t) header[7] << 8) + ((uint32_t) header[6] << 16) + ((uint32_t) header[5] << 24);
      // send ACK packet
      return STATE_STEADY;
    }
    else {
    }
  }
  Serial.println("Searching for a parent node.");
  messageBuffer[0] = 2;
  for (int i = 0; i < 4; i++) messageBuffer[1+i] = networkID[i];
  for (int i = 5; i < 9; i++) messageBuffer[i] = 0;
  schedule_broadcast(messageBuffer, 9);
  delay(1000);
  return STATE_SEARCH;
}
