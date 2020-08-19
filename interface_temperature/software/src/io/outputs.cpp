/**
  * @file   output.cpp
  * @brief  Handle logic output
  * @author David DEVANT
  * @date   12/08/2019
  */

#define IO_OUTPUTS_CPP

#include "outputs.hpp"
#include "global.hpp"

#ifdef MODULE_OUTPUTS

struct output_t {
	uint8_t  pin;
	bool     state;
	uint32_t timeout;
	bool     delayedState;
};

// VARIABLES
extern uint32_t        tick;
static struct output_t outputData[OUTPUTS_COUNT] = { 0 };

void outputs_init(void)
{
	const uint8_t outputPins[OUTPUTS_COUNT] = OUTPUTS_PINS;

	for (uint8_t i = 0; i < OUTPUTS_COUNT; i++) {
		// Save the pin into the structure
		outputData[i].pin = outputPins[i];

		// Configure the pin as output
		pinMode(outputData[i].pin, OUTPUT);
	}
}

void output_set(uint8_t i, bool state)
{
	outputData[i].state = state;
	digitalWrite(outputData[i].pin, state);
}

void output_delayed_set(uint8_t i, bool state, uint32_t delay)
{
	if (delay == 0) {
		output_set(i, state);
	} else {
		outputData[i].timeout      = tick + delay;
		outputData[i].delayedState = state;
	}
}

bool output_get(uint8_t i)
{
	return outputData[i].state;
}

void output_main(void)
{
	for (uint8_t i = 0; i < OUTPUTS_COUNT; i++) {
		if (tick >= outputData[i].timeout) {
			// Reset timeout
			outputData[i].timeout = UINT32_MAX;

			// Define the new output state
			output_set(outputData[i].pin, outputData[i].delayedState);
		}
	}
}

#endif /* MODULE_OUTPUTS */