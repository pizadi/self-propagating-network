#include "utils.h"

int compareBytes(byte * src, byte * trg, uint32_t len) {
  for (int i = 0; i < len; i++) {
    if (src[i] != trg[i]) return false;
  }
  return true;
}
