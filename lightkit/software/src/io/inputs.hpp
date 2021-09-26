/**
 * @file inputs.hpp
 * @brief Handle logic and analog inputs
 * @author David DEVANT
 * @date 12 / 08 / 2019
 */

#ifndef IO_INPUTS_HPP
#define IO_INPUTS_HPP

#include "io.hpp"

#ifdef MODULE_INPUTS

/* INPUT_STATE */
#define INPUT_STATE_RISING    0x01
#define INPUT_STATE_FALLING   0x02
#define INPUT_STATE_LONG_HIGH 0x04
#define INPUT_STATE_LONG_LOW  0x08
#define INPUT_STATE_IS_HIGH   0x10

// Structures
struct input_t {
	uint32_t pin;
	uint8_t  mode;
	uint8_t  reads;
	uint8_t  state;
	uint32_t risingTick;
	uint32_t fallingTick;
};

/** extern variable used in the macro below */
#ifndef IO_INPUTS_CPP
extern struct input_t inputData[INPUTS_COUNT];
#endif

/** MACROS */
#define is_input_high(i) _isset(inputData[i].state, INPUT_STATE_IS_HIGH)
#define is_input_low(i)  _isunset(inputData[i].state, INPUT_STATE_IS_HIGH)

#define is_input_long_high(i)    _isset(inputData[i].state, INPUT_STATE_LONG_HIGH)
#define is_input_long_low(i)     _isset(inputData[i].state, INPUT_STATE_LONG_LOW)
#define reset_input_long_high(i) _unset(inputData[i].state, INPUT_STATE_LONG_HIGH)
#define reset_input_long_low(i)  _unset(inputData[i].state, INPUT_STATE_LONG_LOW)

#define is_input_falling(i)    _isset(inputData[i].state, INPUT_STATE_FALLING)
#define is_input_rising(i)     _isset(inputData[i].state, INPUT_STATE_RISING)
#define reset_input_falling(i) _unset(inputData[i].state, INPUT_STATE_FALLING)
#define reset_input_rising(i)  _unset(inputData[i].state, INPUT_STATE_RISING)

int      inputs_init();
uint8_t  input_read(uint32_t i);
uint16_t input_analog_read(uint32_t i);
void     inputs_main();

#endif /** MODULE_INPUTS */
#endif /* IO_INPUTS_HPP */