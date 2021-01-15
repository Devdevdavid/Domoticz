/**
  * @file   inputs.hpp
  * @brief  Handle logic and analog inputs
  * @author David DEVANT
  * @date   12/08/2019
  */

#include "global.hpp"

#ifndef IO_INPUTS_HPP
#define IO_INPUTS_HPP

#ifdef MODULE_INPUTS

/* INPUT_STATE */
#define INPUT_STATE_RISING    0x01
#define INPUT_STATE_FALLING   0x02
#define INPUT_STATE_LONG_HIGH 0x04
#define INPUT_STATE_LONG_LOW  0x08
#define INPUT_STATE_IS_HIGH   0x10

/** extern variable used in the macro below */
#ifndef IO_INPUTS_CPP
extern uint8_t inputStates[INPUTS_COUNT];
#endif

/** MACROS */
#define is_input_high(i) _isset(inputStates[i], INPUT_STATE_IS_HIGH)
#define is_input_low(i)  _isunset(inputStates[i], INPUT_STATE_IS_HIGH)

#define is_input_long_high(i)    _isset(inputStates[i], INPUT_STATE_LONG_HIGH)
#define is_input_long_low(i)     _isset(inputStates[i], INPUT_STATE_LONG_LOW)
#define reset_input_long_high(i) _unset(inputStates[i], INPUT_STATE_LONG_HIGH)
#define reset_input_long_low(i)  _unset(inputStates[i], INPUT_STATE_LONG_LOW)

#define is_input_falling(i)    _isset(inputStates[i], INPUT_STATE_FALLING)
#define is_input_rising(i)     _isset(inputStates[i], INPUT_STATE_RISING)
#define reset_input_falling(i) _unset(inputStates[i], INPUT_STATE_FALLING)
#define reset_input_rising(i)  _unset(inputStates[i], INPUT_STATE_RISING)

int      inputs_init();
uint16_t input_analog_read(uint8_t i);
void     inputs_main();

#endif /** MODULE_INPUTS */
#endif /* IO_INPUTS_HPP */