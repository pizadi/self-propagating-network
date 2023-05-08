#include "states.h"
#include "globals.h"

int func_steady() {  if (inputFIFO_len > 0) {
    byte header[16];
    parseHead(header, HEADER_ADP);
    if (header[0]) {
      
    }
  }
  return STATE_STEADY;
}
