#define MAIN_C

#include "global.hpp"
#include "bootloader/bootloader.hpp"
#include "bootloader/file_sys.hpp"
#include "temp/temp.hpp"
#include "button/button.hpp"
#include "web/web_server.hpp"
#include "stripLed/stripLed.hpp"
#include "status_led/status_led.hpp"
#include "cmd/cmd_serial.hpp"
#include "relay/relay.hpp"
#include "script/script.hpp"

uint32_t tick, curTick;

void setup()
{
	Serial.begin(115200);
	Serial.print("\n\r\n\r"); // Jump some lines after internal firmware stuff
	log_info("Starting %s", FIRMWARE_VERSION);

	status_init();
	file_sys_init();
#ifdef MODULE_BUTTON
		button_init();
#endif
#ifdef MODULE_STATUS_LED
	status_led_init();
#endif
	bootloader_init();
#ifdef MODULE_WEBSERVER
	web_server_init();
#endif
#ifdef MODULE_TEMPERATURE
	temp_init();
#endif
#ifdef MODULE_STRIPLED
	stripLed_init();
#endif
#ifdef MODULE_RELAY
		relay_init();
#endif
#ifdef MODULE_CMD_SERIAL
	cmd_serial_init();
#endif
}

void loop(void) 
{
	curTick = millis();

	bootloader_main();
#ifdef MODULE_WEBSERVER
	web_server_main();
#endif

	if (tick != curTick) {
		tick = curTick;
#ifdef MODULE_BUTTON
		button_main();
#endif
#ifdef MODULE_TEMPERATURE
		temp_main();
#endif
#ifdef MODULE_STRIPLED
		stripLed_main();
#endif
#ifdef MODULE_CMD_SERIAL
		cmd_serial_main();
#endif
#ifdef MODULE_STATUS_LED
		status_led_main();
#endif
#ifdef MODULE_RELAY
		relay_main();
#endif
		script_execute();
	}
}
