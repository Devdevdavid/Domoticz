#define IO_OUTPUTS_CPP

#include "global.hpp"
#include "outputs.hpp"

#ifdef MODULE_OUTPUTS

const uint8_t outputPins[OUTPUTS_COUNT] = OUTPUTS_PINS;
uint8_t outputStates[OUTPUTS_COUNT] = {0};

void outputs_init(void)
{
	for (uint8_t i = 0; i < OUTPUTS_COUNT; i++) {
    	pinMode(outputPins[i], OUTPUT);
	}
}

void output_set(uint8_t i, bool state)
{
	outputStates[i] = state;
	digitalWrite(outputPins[i], outputStates[i]);
}

#endif /* MODULE_OUTPUTS */