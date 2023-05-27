#include "headers.h"
#include "parser.h"
#include "globals.h"
#include "utils.h"
#include <esp32/rom/crc.h>

int parseHead(byte * out, uint8_t flags = 0xFF) {
  uint8_t rem = 0;
  while (inputFIFO_len > 24) {
    inputFIFO_len -= rem;
    inputFIFO_head = (inputFIFO_head + rem) % MAX_INPUT_FIFO;
    rem = 0;
    
    uint8_t packlen = inputFIFO[inputFIFO_head];
    if (packlen % 16 != 0) { // In case that the FIFO is desynchronized
      rem = 1;
      continue;
    }

    if (packlen + 8 > inputFIFO_len) return 0; // In case that the packet has not yet been received

    byte packet[248];
    for (int i = 0; i < packlen+8; i++) packet[i] = inputFIFO[(inputFIFO_head + i) % MAX_INPUT_FIFO];

    if (!compareBytes(packet+1, networkID, 4)) { // In case that netKey is wrong
      rem = 1;
      continue;
    }

    byte decpack[248];
    decrypt(packet, packet+8, packlen, decpack+8);
    for (int i = 0; i < 8; i++) decpack[i] = packet[i];

    int result = validityCheck(decpack, packlen+8);
    if (result == 0) {
      rem = packlen;
    }
    else if (result == -1) {
      rem = 1;
    }
    else if (decpack[8] & flags){
      for (int i = 0; i < packlen+8; i++) out[i] = decpack[i];
      inputFIFO_len -= packlen + 8;
      inputFIFO_head = (inputFIFO_head + packlen + 8) % MAX_INPUT_FIFO;
      return packlen + 8;
    }
    else {
      rem = packlen;
    }
  }
  return 0;
}

int validityCheck(byte * in, uint8_t len) {
  byte typecode = in[8], device_id[4], timestamp[4], seq[3];
  if (typecode != HEADER_SRCH) {
    for (int i = 0; i < 4; i++) device_id[i] = in[9+i]; 
  }
  for (int i = 0; i < 4; i++) timestamp[i] = in[len-i-1];
  for (int i = 0; i < 3; i++) seq[3] = packet[5+i]

  if (typecode != HEADER_MSG && typecode != HEADER_CHECK && typecode != HEADER_ACK && typecode != HEADER_CMD && typecode != HEADER_SRCH && typecode != HEADER_ADP) return -1;
  if (typecode == HEADER_MSG) {
    byte plen = in[17], origin_id[4], payload[222], crc32[4];
    for (int i = 0; i < 4; i++) origin_id[i] = in[13+i];

    if (plen > len - 26) return -1;

    for (int i = 0; i < plen; i++) payload[i] = in[18+i];
    for (int i = 0; i < 4; i++) crc[i] = in[22+i];

    if (!idCheck(device_id)) return -1;
    if (!idCheck(origin_id)) return -1;
    if (!crcCheck(payload, plen, crc32)) return -1;
    for (int i = plen+22; i < len-4; i++) if (in[i] != 0) return -1;
  }

  if (typecode == HEADER_CHECK) {
    if (!idCheck(device_id)) return -1;
    for (int i = 13; i < len-4; i++) if (in[i] != 0) return -1;
  }

  if (typecode == HEADER_ACK) {
    byte seq_number[3];
    for (int i = 0; i < 3; i++) seq_number[i] = packet[13+i];
    if (!idCheck(device_id)) return -1;
    for (int i = 16; i < len-4; i++) if (in[i] != 0) return -1;
  }

  if (typecode == HEADER_CMD) {
    byte plen = in[17], target_id[4], payload[222], crc32[4];
    for (int i = 0; i < 4; i++) target_id[i] = in[13+i];

    if (plen > len - 26) return -1;

    for (int i = 0; i < plen; i++) payload[i] = in[18+i];
    for (int i = 0; i < 4; i++) crc[i] = in[22+i];

    if (!idCheck(device_id)) return -1;
    if (!idCheck(target_id)) return -1;
    if (!crcCheck(payload, plen, crc32)) return -1;
    for (int i = plen+22; i < len-4; i++) if (in[i] != 0) return -1;
    
  }

  if (typecode == HEADER_SRCH) {
    byte temp_id[2];
    for (int i = 0; i < 2; i++) temp_id[i] = packet[9+i];

  }

  if (typecode == HEADER_ADP) {
    byte temp_id[2];
    for (int i = 0; i < 2; i++) temp_id[i] = packet[13+i];

  }
  if (typecode != HEADER_SRCH && !childCheck(device_id)) return 0;
  if (!timeCheck(timestamp)) return 0;
  if (!seqCheck(seq, device_id)) return 0;
  return typecode;
}

void decrypt(byte * salt, byte * src, uint8_t len, byte * out) {
  // Key generation
  byte key[32];
  mbedtls_sha256_starts(&shactx, 0);
  mbedtls_sha256_update(&shactx, secret, 30);
  mbedtls_sha256_update(&shactx, salt, 30);
  mbedtls_sha256_finish(&shactx, key);
  // Decryption
  mbedtls_aes_setkey_dec(&aesctx, key, 256);
  byte iv[16];
  for (int i = 0; i < 16; i++) iv[i] = 0;
  mbedtls_aes_crypt_cbc(&aesctx, MBEDTLS_AES_DECRYPT, len, iv, src, out);
}

bool crcCheck(byte * str, uint8_t len, byte * crc) {
  uint32_t romCRC = (~crc32_le((uint32_t)~(0xffffffff), (const uint8_t*)str, len))^0xffffffff;
  uint32_t inCRC = (uint32_t) crc[0] << 24 + crc[1] << 16 + crc[2] << 8 + crc[3];
  return romCRC == inCRC;
}

bool timeCheck(byte * timest) {
  uint32_t t = 0, control = rtc.now().unixtime();
  for (int i = 0; i < 4; i++) t = (t << 8);
  if (t <= control && t + MAX_RTC_TIMEOUT >= control) return true;
  else false;  
}

bool idCheck(byte * id) {
  bool flag = false;
  for (int i = 0; i < 16; i++) {
    uint8_t t = id[i/4] & (0xC0 >> (2*(i%4)));
    if (!flag && t == 0) flag = true;
    else if (flag && t > 0) return false;
  }
  return true;
}

bool childCheck(byte * id) {

}