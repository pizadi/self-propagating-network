#include <Preferences.h>
#include "states.h"
#include "globals.h"


int func_search() {
  byte bestCandidate[4] = {0x00, 0x00, 0x00, 0x00};
  byte header[248];
  while (true){
    int result = parseHead(header, HEADER_ADP);
    if (result > 0) {
      Serial.println("Adopt packet received.");
    }
    else break;
    for (int i = 0; i < 4; i++) bestCandidate[i] = header[i+9];
    setID(bestCandidate);
    return STATE_STEADY;
  }

  Serial.println("Searching for a parent node.");
  messageBuffer[0] = HEADER_SRCH;
  for (int i = 0; i < 4; i++) messageBuffer[1+i] = networkID[i];
  messageBuffer[5] = tempID >> 8;
  messageBuffer[6] = tempID;
  schedule_broadcast(messageBuffer, 7);
  delay(10000);
  return STATE_SEARCH;
}

void setID(byte * bestID) {
  for (int i = 0; i < 4; i++) deviceID[i] = bestID[i];
  // Create and send CHECK packet
}