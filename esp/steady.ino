#include "states.h"
#include "globals.h"
#include "steady.h"
#include "transmission.h"

int func_steady() {
  byte packet[248];
  int result = parseHead(packet, HEADER_ANY ^ HEADER_ADP);
  if (result == HEADER_MSG) {
    handleMSG(packet);
  }
  else if (result == HEADER_CMD) {
    handleCMD(packet);
  }
  else if (result == HEADER_ACK) {
    handleACK(packet);
  }
  else if (result == HEADER_SRCH) {
    handleSRCH(packet);
  }

  updateNodes();

  return STATE_STEADY;
}

void handleMSG(byte * packet) {
  uint8_t l = packet[0] + 8;
  byte newpack[248], ackPack[24];
  createACK(ackPack, packet+9, packet+5);
  schedule_broadcast(ackPack, 24);
  for (int i = 0; i < l; i++) newpack[i] = packet[i];
  fetchNextHopMSG(newpack);
  for (int i = 0; i < 3; i++) newpack[i] = 0;
  for (int i = 0; i < 4; i++) newpack[l-1-i] = 0;
  schedule_broadcast(newpack, l);
}

void handleCMD(byte * packet) {
  if (compareBytes(packet + 13, deviceID, 4)) {
    // TODO: Code to handle incoming commands
  }
  else {
    uint8_t l = packet[0] + 8;
    byte newpack[248], ackPack[24];
    createACK(ackPack, packet+9, packet+5);
    schedule_broadcast(ackPack, 24);
    for (int i = 0; i < l; i++) newpack[i] = packet[i];
    fetchNextHopCMD(newpack);
    for (int i = 0; i < 3; i++) newpack[i] = 0;
    for (int i = 0; i < 4; i++) newpack[l-1-i] = 0;
    schedule_broadcast(newpack, l);
  }
}

void handleACK(byte * packet) {
  uint32_t ackseq = 0;
  for (int i = 2; i >= 0; i++) ackseq = packet[15-i] + (ackseq << 8);
  if (broadcastFIFO[broadcastFIFO_head].seq == ackseq) {
    inputFIFO_len--;
    inputFIFO_head = (inputFIFO_head + 1) % MAX_INPUT_FIFO;
  }
}

void handleSRCH(byte * packet) {
  int i = 1;
  for (; i < 4; i++) {
    if (neighbors[i].lastTransmission == 0) break;
  }
  if (i >= 4) return;
  byte adpPack[24]; 
  createADP(adpPack, i, packet+9);
  schedule_broadcast(adpPack, 24); 
}

void fetchNextHopCMD(byte * packet) {
  byte * tp = packet + 13;
  byte * np = packet + 9;
  for (int i = 0; i < 16; i++) {
    uint8_t t = tp[i/4] & (0b11 << (2 * (i%4))), n = np[i/4] & (0b11 << (2 * (i%4)));
    if (n == 0) {
      np[i/4] = np[i/4] | t;
      break;
    }
  }
}

void fetchNextHopMSG(byte * packet) {
  byte * np = packet + 9;
  for (int i = 0; i < 16; i++) {
    uint8_t n = np[i/4] & (0b11 << (2 * (i%4)));
    if (n == 0) {
      np[(i-1)/4] = np[(i-1)/4] & (~(0b11 << (2 * ((i-1)%4))));
      break;
    }
  }
}

void updateNodes() {
  for (int i = 0; i < 4; i++) {
    uint32_t tnow = rtc.now().unixtime();
    if (neighbors[i].lastTransmission > 0 &&(neighbors[i].timeouts > 10 || neighbors[i].lastTransmission < tnow)) {
      neighbors[i].lastTransmission = 0;
      if (i == 0) STATE = STATE_SEARCH;
    }
  }
}
