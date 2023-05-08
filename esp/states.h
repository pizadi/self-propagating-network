#ifndef STATES_H
#define STATES_H

// STATE DEFINES

#define STATE_SLEEP 0
#define STATE_BOOT 1
#define STATE_SEARCH 2
#define STATE_STEADY 3

// STATE FUNCTIONS

int func_sleep();
int func_boot();
int func_searching();
int func_steady();

// CONFIG INTERRUPT

void IRAM_ATTR func_config();

#endif
