#include "states.h"
#include "globals.h"

int func_steady() {  if (inputFIFO_len > 0) {
    byte * header = parseHead(HEADER_ADP);
    if (header && header[0]) {
      
    }
  }
  return STATE_STEADY;
}
