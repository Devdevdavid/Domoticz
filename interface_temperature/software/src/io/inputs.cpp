/**
  * @file   inputs.cpp
  * @brief  Handle logic and analog inputs
  * @author David DEVANT
  * @date   12/08/2019
  */

#define IO_INPUTS_CPP

#include "inputs.hpp"
#include "global.hpp"

#ifdef MODULE_INPUTS

extern uint32_t tick;
const uint8_t   inputPins[INPUTS_COUNT]        = INPUTS_PINS;
const uint8_t   inputModes[INPUTS_COUNT]       = INPUTS_MODES;
uint8_t         inputReads[INPUTS_COUNT]       = { 0 };
uint8_t         inputStates[INPUTS_COUNT]      = { 0 };
uint32_t        inputRisingTick[INPUTS_COUNT]  = { 0 };
uint32_t        inputFallingTick[INPUTS_COUNT] = { 0 };

void inputs_init(void)
{
	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
		switch (inputModes[i]) {
		case N:
			pinMode(inputPins[i], INPUT);
			break;
		case U:
			pinMode(inputPins[i], INPUT_PULLUP);
			break;
		case A:
			/* Analog pins are init in analogRead() */
			break;
		default:
			log_error("Unkwnon input mode: %d", inputModes[i]);
			break;
		}
	}
}

uint16_t input_analog_read(uint8_t i)
{
	// Integrity check
	if ((inputModes[i] != A) || (i >= INPUTS_COUNT)) {
		return 0xFFFF;
	}

	return (uint16_t) analogRead(inputPins[i]);
}

void inputs_main(void)
{
	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
		// Do not read Analog pins
		if (inputModes[i] == A) {
			continue;
		}

		// Shift 1 bit
		inputReads[i] <<= 1;

		// Read input
		if (digitalRead(inputPins[i])) {
			inputReads[i] |= 1;
		}

		// High
		if (inputReads[i] == 0xFF) {
			_set(inputStates[i], INPUT_STATE_IS_HIGH);
			if (tick >= (inputRisingTick[i] + INPUTS_LONG_HOLD_TIME)) {
				_set(inputStates[i], INPUT_STATE_LONG_HIGH);
				inputRisingTick[i] = 0;
			}
		}
		// Low
		else if (inputReads[i] == 0x00) {
			_unset(inputStates[i], INPUT_STATE_IS_HIGH);
			if (tick >= (inputFallingTick[i] + INPUTS_LONG_HOLD_TIME)) {
				_set(inputStates[i], INPUT_STATE_LONG_LOW);
				inputFallingTick[i] = 0;
			}
		}
		// Rising
		else if (inputReads[i] == 0x0F) {
			inputRisingTick[i] = tick;
			_set(inputStates[i], INPUT_STATE_RISING);
		}
		// Falling
		else if (inputReads[i] == 0xF0) {
			inputFallingTick[i] = tick;
			_set(inputStates[i], INPUT_STATE_FALLING);
		}
	}
}

#endif /* MODULE_INPUTS */