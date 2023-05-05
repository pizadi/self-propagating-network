#include "states.h"
#include "globals.h"

int func_sleep() {
  delay(100000);
  return STATE_SLEEP;
}
