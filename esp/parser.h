#ifndef PARSER_H
#define PARSER_H

int parseHead(byte * out, uint8_t flags);
int validityCheck(byte * in, uint8_t len, byte * out);
void decrypt(byte * salt, byte * src, uint8_t len, byte * out);
bool crcCheck(byte * str, uint8_t len, byte * crc);
bool timeCheck(byte * timest);
bool idCheck(byte * id);
bool seqCheck(byte * seq, byte * device_id);
bool childCheck(byte * id);
#endif
