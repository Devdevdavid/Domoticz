/*
 * @file   inputs.cpp
 * @brief  Handle logic and analog inputs
 * @author David DEVANT
 * @date   12/08/2019
 */

#define IO_INPUTS_CPP

#include "inputs.hpp"

#ifdef MODULE_INPUTS

// Variables
extern uint32_t tick;
static uint32_t inputNextTick           = 0;
struct input_t  inputData[INPUTS_COUNT] = { 0 };

int inputs_init(void)
{
	const uint32_t inputPins[INPUTS_COUNT]  = INPUTS_PINS;
	const uint8_t  inputModes[INPUTS_COUNT] = INPUTS_MODES;

	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
		// We do a hard copy here
		inputData[i].pin  = inputPins[i];
		inputData[i].mode = inputModes[i];

		if (is_io_special_none(inputData[i].pin)) {
			switch (inputData[i].mode) {
			case I_N:
				pinMode(inputData[i].pin, INPUT);
				break;
			case I_U:
				pinMode(inputData[i].pin, INPUT_PULLUP);
				break;
			case I_A:
				/* Analog pins are init in analogRead() */
				break;
			default:
				log_error("Unkwnon input mode: %d", inputModes[i]);
				break;
			}
		}
	}

	return 0;
}

uint8_t input_read(uint32_t specialPin)
{
	if (is_io_special_none(specialPin)) {
		return digitalRead(specialPin);
	}

#ifdef HAS_IOI2C_BOARD
	int8_t read;

	if (is_io_special_ioi2c(specialPin)) {
		read = ioi2cGroup.input_read(io_special_get_pin(specialPin));
		if (read < 0) {
			log_error("Unable to read input from IOI2C device");
			return 0;
		}
		return (uint8_t) read;
	}
#endif

	log_warn("Unsupported special input function");
	return 0;
}

uint16_t input_analog_read(uint32_t i)
{
	// Integrity check
	if ((inputData[i].mode != I_A) || (i >= INPUTS_COUNT)) {
		return 0xFFFF;
	}

	if (!is_io_special_none(inputData[i].pin)) {
		return 0xFFFF;
	}

	return (uint16_t) analogRead(io_special_get_pin(inputData[i].pin));
}

void inputs_main(void)
{
	if (tick < inputNextTick) {
		return;
	}
	inputNextTick = tick + 10;

	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
		// Do not read Analog pins
		if (inputData[i].mode == I_A) {
			continue;
		}

		// Shift 1 bit
		inputData[i].reads <<= 1;

		// Read input
		if (input_read(inputData[i].pin)) {
			inputData[i].reads |= 1;
		}

		// High
		if (inputData[i].reads == 0xFF) {
			_set(inputData[i].state, INPUT_STATE_IS_HIGH);
			if (tick >= (inputData[i].risingTick + INPUTS_LONG_HOLD_TIME)) {
				_set(inputData[i].state, INPUT_STATE_LONG_HIGH);
				inputData[i].risingTick = 0;
			}
		}
		// Low
		else if (inputData[i].reads == 0x00) {
			_unset(inputData[i].state, INPUT_STATE_IS_HIGH);
			if (tick >= (inputData[i].fallingTick + INPUTS_LONG_HOLD_TIME)) {
				_set(inputData[i].state, INPUT_STATE_LONG_LOW);
				inputData[i].fallingTick = 0;
			}
		}
		// Rising
		else if (inputData[i].reads == 0x0F) {
			inputData[i].risingTick = tick;
			_set(inputData[i].state, INPUT_STATE_RISING);
		}
		// Falling
		else if (inputData[i].reads == 0xF0) {
			inputData[i].fallingTick = tick;
			_set(inputData[i].state, INPUT_STATE_FALLING);
		}
	}
}

#endif /* MODULE_INPUTS */