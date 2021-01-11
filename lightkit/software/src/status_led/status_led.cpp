/**
  * @file   status_led.cpp
  * @brief  Handle animation for led status
  * @author David DEVANT
  * @date   12/08/2017
  */

#include "status_led.hpp"
#include "cmd/cmd.hpp"

#ifdef MODULE_STATUS_LED

extern uint32_t     tick;
struct status_led_t status_led[STATUS_LED_NB_LED];

void update_states(void)
{
	uint8_t index;

	for (index = 0; index < STATUS_LED_NB_LED; index++) {
		if (tick >= status_led[index].nextChange) {
			status_led[index].state.hasChanged = true;
			if (status_led[index].state.isOn) {
				if (status_led[index].timeOff != 0) {
					status_led[index].nextChange = tick + status_led[index].timeOff;
					status_led[index].state.isOn = false;
				} else {
					status_led[index].nextChange = tick + status_led[index].timeOn;
				}
			} else {
				if (status_led[index].timeOn != 0) {
					status_led[index].nextChange = tick + status_led[index].timeOn;
					status_led[index].state.isOn = true;
				} else {
					status_led[index].nextChange = tick + status_led[index].timeOff;
				}
			}
		}
	}
}

#if STATUS_LED_TYPE == STATUS_LED_TYPE_CLASSIC

void status_led_turnoff(void)
{
	for (index = 0; index < STATUS_LED_NB_LED; index++) {
		digitalWrite(status_led[index].pin, LOW);
	}
}

void status_led_init(void)
{
	status_led[0].pin = STATUS_LED_POWER_PIN;
	status_led[1].pin = STATUS_LED_FAULT_PIN;

	for (uint8_t index = 0; index < STATUS_LED_NB_LED; index++) {
		status_led[index].nextChange = 0;
		status_led[index].timeOn     = 500;
		status_led[index].timeOff    = 500;
		status_led[index].isOn       = true;
		pinMode(status_led[index].pin, OUTPUT);
		digitalWrite(status_led[index].pin, LOW);
	}
}

void status_led_main(void)
{
	uint8_t index;

	/* POWER LED */
	if (_isset(STATUS_APPLI, STATUS_APPLI_DEMO_MODE)) {
		status_led[0].timeOn  = 500;
		status_led[0].timeOff = 500;
	} else {
		status_led[0].timeOn  = 100;
		status_led[0].timeOff = 0;
	}

	/* FAULT LED */
	if (!_isset(STATUS_WIFI, STATUS_WIFI_IS_CO)) {
		status_led[1].timeOn  = 300;
		status_led[1].timeOff = 500;
	} else if (_isset(STATUS_TEMP, STATUS_TEMP_1_FAULT | STATUS_TEMP_2_FAULT)) {
		status_led[1].timeOn  = 200;
		status_led[1].timeOff = 1800;
	} else if (_isset(STATUS_APPLI, STATUS_APPLI_ERROR) || _isset(STATUS_APPLI, STATUS_APPLI_FILESYSTEM)) {
		status_led[1].timeOn  = 100;
		status_led[1].timeOff = 0;
	} else {
		status_led[1].timeOn  = 0;
		status_led[1].timeOff = 100;
	}

	update_states();

	if (_isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED)) {
		for (index = 0; index < STATUS_LED_NB_LED; index++) {
			if (status_led[index].state.hasChanged) {
				digitalWrite(status_led[index].pin, (status_led[index].state.isOn) ? LOW : HIGH);
				status_led[index].state.hasChanged = false;
			}
		}
	}
}

#elif STATUS_LED_TYPE == STATUS_LED_TYPE_NEOPIXEL

#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_INDEX 0

Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel(STATUS_LED_NB_LED, STATUS_LED_NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/**
 * @brief Define the color of the neopixel status LED
 * @param color
 * '-': Turn off LED
 * 'r': Show red
 * 'g': Show green
 * 'b': Show blue
 * 'w': Show white
 */
void status_led_set_color(char color)
{
	uint8_t r = 0, g = 0, b = 0;

	switch (color) {
	case '-':
		break;
	case 'r':
		r = STATUS_LED_NEOPIXEL_RED;
		break;
	case 'g':
		g = STATUS_LED_NEOPIXEL_GREEN;
		break;
	case 'b':
		b = STATUS_LED_NEOPIXEL_BLUE;
		break;
	case 'w':
		r = 255;
		g = 255;
		b = 255;
		break;
	default:
		return;
	}
	// It seems the library has a bug, or the led is wired diferently
	// I need to exchange red and green value to get the expected result
	neoPixel.setPixelColor(NEOPIXEL_INDEX, g, r, b);
	neoPixel.show();
}

void status_led_turnoff(void)
{
	status_led_set_color('-');
}

void status_led_init(void)
{
	neoPixel.begin();
	neoPixel.setBrightness(STATUS_LED_NEOPIXEL_BRIGHTNESS);
	cmd_set_status_led(true);
}

void status_led_main(void)
{
	uint8_t index;

	if (!_isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED)) {
		// Status led disabled
		status_led[0].timeOn  = 0;
		status_led[0].timeOff = 500;
		status_led[0].color   = '-';
	} else if (_isset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT) || _isset(STATUS_TEMP, STATUS_TEMP_1_FAULT | STATUS_TEMP_2_FAULT)) {
		// Hardware Fault
		status_led[0].timeOn  = 100;
		status_led[0].timeOff = 900;
		status_led[0].color   = 'r';
	} else if (_isset(STATUS_WIFI, STATUS_WIFI_DOMOTICZ_FAULT) || _isunset(STATUS_WIFI, STATUS_WIFI_IS_CO)) {
		// Wireless fault
		status_led[0].timeOn  = 100;
		status_led[0].timeOff = 900;
		status_led[0].color   = 'b';
	} else if (_isset(STATUS_APPLI, STATUS_APPLI_ERROR)) {
		status_led[0].timeOn  = 100;
		status_led[0].timeOff = 0;
		status_led[0].color   = 'r';
	} else {
		status_led[0].timeOn  = 100;
		status_led[0].timeOff = 0;
		status_led[0].color   = 'g';
	}

	update_states();

	for (index = 0; index < STATUS_LED_NB_LED; index++) {
		if (status_led[index].state.hasChanged) {
			if (status_led[index].state.isOn) {
				status_led_set_color(status_led[0].color);
			} else {
				status_led_set_color('-');
			}
			status_led[index].state.hasChanged = false;
		}
	}
}
#endif

#endif /* MODULE_STATUS_LED */