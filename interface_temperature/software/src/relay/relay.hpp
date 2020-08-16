#ifndef RELAY_RELAY_HPP
#define RELAY_RELAY_HPP

#include <Arduino.h>

bool relay_get_state(void);
void relay_set_toogle_timeout(uint32_t timeoutMs);
void relay_set_state(bool isClose);
bool relay_toogle_state(void);
void relay_init(void);
void relay_main(void);

#endif /* RELAY_RELAY_HPP */