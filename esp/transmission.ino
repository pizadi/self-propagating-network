#include "transmission.h"
#include "globals.h"
#include <cstdint>

uint8_t createCHECK(byte * out, uint8_t trg) {
    out[0] = 16;
    for (int i = 0; i < 4; i++) out[1+i] = networkID[i];
    out[8] = HEADER_CHECK;
    byte trgID[4];
    getID(trg, trgID);
    for (int i = 0; i < 4; i++) out[9+i] = trgID[i];
    for (int i = 13; i < 24; i++) out[i] = 0;
    return 24;
}

uint8_t createMSG(byte * out, byte * payload, uint8_t len, byte * origin = NULL) {
    uint8_t tlen = len+18;
    if (tlen % 16 > 0) tlen += 16 - (tlen % 16);
    out[0] = tlen;
    for (int i = 0; i < 4; i++) out[1+i] = networkID[i];
    out[8] = HEADER_MSG;
    for (int i = 0; i < 4; i++) out[i+9] = deviceID[i];
    byte parentID[4];
    getParent(parentID);
    for (int i = 0; i < 4; i++) out[i+9] = parentID[i];
    uint32_t romCRC = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)payload, len))^0xffffffff;
    for (int i = 3; i >= 0; i++) {
      out[10+len+i] = romCRC & 0xFF;
      romCRC = romCRC >> 8; 
    }
    return tlen + 8;

}

uint8_t createCMD(byte * out, byte * payload, uint8_t len) {
    uint8_t tlen = len+18;
    if (tlen % 16 > 0) tlen += 16 - (tlen % 16);
    out[0] = tlen;
    for (int i = 0; i < 4; i++) out[1+i] = networkID[i];
    out[8] = HEADER_MSG;
    for (int i = 0; i < 4; i++) out[i+9] = deviceID[i];
    byte parentID[4];
    getParent(parentID);
    for (int i = 0; i < 4; i++) out[i+9] = parentID[i];
    uint32_t romCRC = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)payload, len))^0xffffffff;
    for (int i = 3; i >= 0; i++) {
      out[10+len+i] = romCRC & 0xFF;
      romCRC = romCRC >> 8; 
    }
    return tlen + 8;
}

uint8_t createACK(byte * out, byte * trg, byte * seq) {
    out[0] = 16;
    for (int i = 0; i < 4; i++) out[1+i] = networkID[i];
    out[8] = HEADER_ACK;
    byte trgID[4];
    for (int i = 0; i < 4; i++) out[9+i] = trg[i];
    for (int i = 0; i < 3; i++) out[13+i] = seq[i];
    for (int i = 0; i < 8; i++) out[15+i] = 0;
    return 24;
}

uint8_t createADP(byte * out, uint8_t trg, byte * tmp) {
    out[0] = 16;
    for (int i = 0; i < 4; i++) out[1+i] = networkID[i];
    out[8] = HEADER_ADP;
    byte trgID[4];
    getID(trg, trgID);
    for (int i = 0; i < 4; i++) out[9+i] = trgID[i];
    for (int i = 0; i < 2; i++) out[13+i] = tmp[i];
    for (int i = 0; i < 9; i++) out[15+i] = 0;
    return 24;
}

void getParent(byte * parentID) {
    for (int i = 0; i < 4; i++) parentID[i] = deviceID[i];
    for (int i = 0; i < 16; i++) {
        uint8_t n = parentID[i/4] & (0b11000000 >> (2 * (i%4)));
        if (n == 0) {
            parentID[(i-1)/4] = parentID[(i-1)/4] & (~(0b11000000 >> (2 * ((i-1)%4))));
            break;
        }
    }
}

void getID(uint8_t trg, byte * trgID) {
  if (trg == 0) getParent(trgID);
  else {
    for (int i = 0; i < 4; i++) trgID[i] = deviceID[i];
    for (int i = 0; i < 16; i++) {
      uint8_t n = trgID[i/4] & (0b11000000 >> (2 * (i%4)));
      if (n == 0) {
        trgID[i/4] = trgID[i/4] | (trg >> (6 - 2 * (i%4)));
      }
    }
  }
}
