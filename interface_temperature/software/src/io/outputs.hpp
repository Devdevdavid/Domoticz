#include <Arduino.h>

#ifndef IO_OUTPUTS_HPP
#define IO_OUTPUTS_HPP

#ifdef MODULE_OUTPUTS

void outputs_init();
void output_set(uint8_t i, bool state);
void output_delayed_set(uint8_t i, bool state, uint32_t delay);
bool output_get(uint8_t i);
void output_main(void);

#endif /** MODULE_OUTPUTS */
#endif /* IO_OUTPUTS_HPP */