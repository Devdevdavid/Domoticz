/**
  * @file   output.cpp
  * @brief  Handle logic output
  * @author David DEVANT
  * @date   12/08/2019
  */

#define IO_OUTPUTS_CPP

#include "outputs.hpp"

#ifdef MODULE_OUTPUTS

struct output_t {
	uint32_t pin;
	bool     state;
	uint32_t timeout;
	bool     delayedState;
};

// VARIABLES
extern uint32_t        tick;
static struct output_t outputData[OUTPUTS_COUNT] = { 0 };

int outputs_init(void)
{
	const uint32_t outputPins[OUTPUTS_COUNT] = OUTPUTS_PINS;

	for (uint8_t i = 0; i < OUTPUTS_COUNT; i++) {
		// Save the pin into the structure
		outputData[i].pin = outputPins[i];

		if (is_io_special_none(outputData[i].pin)) {
			// Configure the pin as output
			pinMode(outputData[i].pin, OUTPUT);
		}
	}

#ifdef HAS_IOI2C_BOARD
	ioi2cGroup.begin();
#endif

	return 0;
}

void output_set(uint32_t i, bool state)
{
	outputData[i].state = state;

	if (is_io_special_none(outputData[i].pin)) {
		digitalWrite(outputData[i].pin, state);
		return;
	}

#ifdef HAS_IOI2C_BOARD
	if (is_io_special_ioi2c(outputData[i].pin)) {
		ioi2cGroup.output_write(io_special_get_pin(outputData[i].pin), state);
		return;
	}
#endif

	log_warn("Unsupported special output function : pin = 0x%02X", outputData[i].pin);
}

void output_delayed_set(uint32_t i, bool state, uint32_t delay)
{
	if (delay == 0) {
		output_set(i, state);
	} else {
		outputData[i].timeout      = tick + delay;
		outputData[i].delayedState = state;
	}
}

bool output_get(uint32_t i)
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
			output_set(i, outputData[i].delayedState);
		}
	}
}

#endif /* MODULE_OUTPUTS */