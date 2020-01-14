#include <Arduino.h>

#ifndef IO_OUTPUTS_HPP
#define IO_OUTPUTS_HPP

#ifdef MODULE_OUTPUTS

void outputs_init();
void output_set(uint8_t i, bool state);

#endif /** MODULE_OUTPUTS */
#endif /* IO_OUTPUTS_HPP */