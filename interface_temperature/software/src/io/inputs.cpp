#define IO_INPUTS_CPP

#include "global.hpp"
#include "inputs.hpp"

#ifdef MODULE_INPUTS

extern uint32_t tick;
const uint8_t inputPins[INPUTS_COUNT] = INPUTS_PINS;
uint8_t inputReads[INPUTS_COUNT] = {0};
uint8_t inputStates[INPUTS_COUNT] = {0};
uint32_t inputRisingTick[INPUTS_COUNT] = {0};
uint32_t inputFallingTick[INPUTS_COUNT] = {0};

void inputs_init(void)
{
	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
    	pinMode(inputPins[i], INPUT_PULLUP);
	}
}

void inputs_main(void)
{
	for (uint8_t i = 0; i < INPUTS_COUNT; i++) {
		// Shift 1 bit
		inputReads[i] <<= 1;

		// Button is active on low
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