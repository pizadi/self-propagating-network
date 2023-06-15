#ifndef TRANSMISSION_H
#define TRANSMISSION_H

uint8_t createCHECK(byte * out, uint8_t trg);
uint8_t createMSG(byte * out, byte * payload);
uint8_t createADP(byte * out, uint8_t trg, byte * tmp);

#endif