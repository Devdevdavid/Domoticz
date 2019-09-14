#include "button.hpp"

#ifdef MODULE_BUTTON

extern uint32_t tick;

void button_init()
{
    pinMode(BUTTON_PIN, INPUT);
}
void button_main()
{
    static uint8_t buttonState = 0;
	static uint32_t pressTick = 0;

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
		if (tick >= (pressTick + BTN_LONG_HOLD_TIME)) {
			_set(STATUS_BUTTON, STATUS_BUTTON_LONG_HOLD);
		}
	}
}

#endif /* MODULE_BUTTON */