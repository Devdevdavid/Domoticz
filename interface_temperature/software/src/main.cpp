/**
  * @file   main.cpp
  * @brief  Entry point of Arduino Platform
  * @author David DEVANT
  * @date   12/08/2019
  */

#define MAIN_C

#include "bootloader/bootloader.hpp"
#include "bootloader/file_sys.hpp"
#include "cmd/cmd_serial.hpp"
#include "flash/flash.hpp"
#include "global.hpp"
#include "io/inputs.hpp"
#include "io/outputs.hpp"
#include "relay/relay.hpp"
#include "script/script.hpp"
#include "status_led/status_led.hpp"
#include "stripled/stripled.hpp"
#include "telegram/telegram.hpp"
#include "temp/temp.hpp"
#include "web/web_server.hpp"

uint32_t tick, curTick;

void setup()
{
	Serial.begin(115200);
	Serial.print("\n\r\n\r"); // Jump some lines after internal firmware stuff
	log_info("Starting %s", FIRMWARE_VERSION);

	status_init();
#ifdef MODULE_FLASH
	flash_init();
#endif
	file_sys_init();
#ifdef MODULE_INPUTS
	inputs_init();
#endif
#ifdef MODULE_OUTPUTS
	outputs_init();
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
	stripled_init();
#endif
#ifdef MODULE_RELAY
	relay_init();
#endif
#ifdef MODULE_TELEGRAM
	telegram_init();
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
#ifdef MODULE_INPUTS
		inputs_main();
#endif
#ifdef MODULE_OUTPUTS
		output_main();
#endif
#ifdef MODULE_TEMPERATURE
		temp_main();
#endif
#ifdef MODULE_STRIPLED
		stripled_main();
#endif
#ifdef MODULE_CMD_SERIAL
		cmd_serial_main();
#endif
#ifdef MODULE_STATUS_LED
		status_led_main();
#endif
#ifdef MODULE_TELEGRAM
		telegram_main();
#endif
#ifdef MODULE_RELAY
		relay_main();
#endif
		script_main();
	}
}
