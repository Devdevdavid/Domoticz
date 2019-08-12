#define MAIN_C

#include "global.hpp"
#include "bootloader/bootloader.hpp"
#include "bootloader/file_sys.hpp"
#include "web/web_server.hpp"
#include "stripLed/stripLed.hpp"
#include "ledVisu/ledVisu.hpp"
#include "cmd/cmd_serial.hpp"

uint32_t tick, curTick;

void gpio_init(void)
{
#if BUTTON_PIN != -1
	pinMode(BUTTON_PIN, INPUT);
#endif 
}

void setup()
{
	status_init();
	gpio_init();
	Serial.begin(115200);
	bootloader_init();
	log_info("Starting %s", FIRMWARE_VERSION);
	file_sys_init();
	web_server_init();
	stripLed_init();
#if (LED_POWER_PIN != -1)
	ledVisu_init();
#endif
	cmd_serial_init();
}

#if BUTTON_PIN != -1
void btn_main(void)
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
		cmd_set_led_visu(true);
	} else if (buttonState == 0xF0) {
		pressTick = tick;
		_set(STATUS_BUTTON, STATUS_BUTTON_FALLING);
	} else if (buttonState == 0xFF) {
		if (tick >= (pressTick + BTN_LONG_HOLD_TIME)) {
			_set(STATUS_BUTTON, STATUS_BUTTON_LONG_HOLD);
			cmd_set_led_visu(false);
		}
	}
}
#endif

void loop(void) 
{
	curTick = millis();

	bootloader_main();
	web_server_main();

	if (tick != curTick) {
		tick = curTick;
#if BUTTON_PIN != -1
		btn_main();
#endif
#if (LED_POWER_PIN != -1)
		ledVisu_main();
#endif
		stripLed_main();
		cmd_serial_main();
	}
}
