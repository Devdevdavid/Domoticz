/**
  * @file   output.hpp
  * @brief  Handle logic output
  * @author David DEVANT
  * @date   12/08/2019
  */

#include <Arduino.h>

#ifndef IO_OUTPUTS_HPP
#define IO_OUTPUTS_HPP

#ifdef MODULE_OUTPUTS

int  outputs_init();
void output_set(uint8_t i, bool state);
void output_delayed_set(uint8_t i, bool state, uint32_t delay);
bool output_get(uint8_t i);
void output_main(void);

#endif /** MODULE_OUTPUTS */
#endif /* IO_OUTPUTS_HPP */