/**
  * @file   main.cpp
  * @brief  Entry point of Arduino Platform
  * @author David DEVANT
  * @date   12/08/2019
  */

#define MAIN_C

#include "cmd/term.hpp"
#include "bootloader/bootloader.hpp"
#include "bootloader/file_sys.hpp"
#include "cmd/serial.hpp"
#include "cmd/telnet.hpp"
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

uint32_t tick = 0, lastTick = 0;

/**
 * @brief Increment tick
 * @details This function is located in IRAM
 * to get better speed performance
 */
void ICACHE_RAM_ATTR tick_interrupt(void)
{
	tick++;
}

static void config_tick(void)
{
	// In both ESP32 and ESP8266, timers are working with a 80MHz clock
#ifdef ESP32
	// We use ESP32 Timer1 (0, 1, 2, 3 exists) to get the same ID as for ESP8266
	hw_timer_t * timer = NULL;
	timer = timerBegin(1, 80, true); // ID timer, prescaler, rising edge
	timerAttachInterrupt(timer, &tick_interrupt, true);
	timerAlarmWrite(timer, 1000, true); // 80MHz / 80 / 1000 = 1kHz
	timerAlarmEnable(timer);
#else
	// Timer0 is used for wifi on ESP8266
	timer1_attachInterrupt(tick_interrupt);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
    timer1_write(5000); // 80MHz / 16 / 5000 = 1kHz
#endif
}

void setup()
{
	config_tick();

#ifdef MODULE_SERIAL
	serial_init();
#endif
#ifdef MODULE_TELNET
	telnet_init();
#endif
#ifdef MODULE_TERM
	term_init();
#endif

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
}

void loop(void)
{
	bootloader_main();
#ifdef MODULE_WEBSERVER
	web_server_main();
#endif

	if (lastTick != tick) {
		lastTick = tick;

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
#ifdef MODULE_SERIAL
		serial_main();
#endif
#ifdef MODULE_TELNET
		telnet_main();
#endif
#ifdef MODULE_TERM
		term_main();
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
