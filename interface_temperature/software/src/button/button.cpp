#include "button.hpp"

#ifdef MODULE_BUTTON

extern uint32_t tick;
uint8_t buttonState = 0;
uint32_t pressTick = 0;

void button_init()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}
void button_main()
{
	// Shift 1 bit
	buttonState <<= 1;

	// Button is active on low
	if (!digitalRead(BUTTON_PIN)) {
		buttonState |= 1;
	}

	if (buttonState == 0x0F) {
		pressTick = tick;
		_set(STATUS_BUTTON, STATUS_BUTTON_RISING);
	} else if (buttonState == 0xF0) {
		pressTick = tick;
		_set(STATUS_BUTTON, STATUS_BUTTON_FALLING);
	} else if (buttonState == 0xFF) {
		_set(STATUS_BUTTON, STATUS_BUTTON_IS_PRESSED);
		if (tick >= (pressTick + BUTTON_LONG_HOLD_TIME)) {
			_set(STATUS_BUTTON, STATUS_BUTTON_LONG_HOLD);
		}
	} else if (buttonState == 0x00) {
		_unset(STATUS_BUTTON, STATUS_BUTTON_IS_PRESSED);
	}
}

#endif /* MODULE_BUTTON */