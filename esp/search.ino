#include <Preferences.h>
#include "states.h"
#include "globals.h"


int func_search() {
  byte bestCandidate[4] = {0x00, 0x00, 0x00, 0x00};
  while (inputFIFO_len > 0) {
    byte header[16];
    header = parseHead(header, HEADER_ADP);
    if (header) {
      // set parent
      overrideID(bestCandidate, header);
      delete[] header;
    }
    else break;
  }
  if (bestCandidate[0] | bestCandidate[1] | bestCandidate[2] | bestCandidate[3]){
    for (int i = 0; i < 4; i++) deviceID[i] = bestCandidate[i];
    setParent();
    messageBuffer[0] = HEADER_CHECK;
    for (int i = 0; i < 4; i++) messageBuffer[1+i] = networkID[i];
    for (int i = 0; i < 4; i++) messageBuffer[5+i] = deviceID[i];
    schedule_broadcast(messageBuffer, 9);
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
