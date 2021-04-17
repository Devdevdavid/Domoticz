/**
  * @file   term.cpp
  * @brief  Terminal management
  * @author David DEVANT
  * @date   19/12/2020
  */

#include "term.hpp"
#include "cmd.hpp"
#include "global.hpp"
#include "relay/relay.hpp"
#include "serial.hpp"
#include "telnet.hpp"

extern "C" {
#include "cli.h"
}

#ifdef MODULE_TERM

//========================
//	  STATIC FUNCTIONS
//========================

/**
 * @brief Default callback for leaf tokens
 *
 * @param argc Argument count
 * @param argv Argument values
 *
 * @return The status of the function
 */
int print_args(uint8_t argc, char * argv[])
{
	printf("print_args() Found %d args:\n\r", argc);
	for (uint8_t i = 0; i < argc; ++i) {
		printf("\t%s\n\r", argv[i]);
	}
	return 0;
}

static int call_reset(uint8_t argc, char * argv[])
{
	cmd_reset_module();
	return 0;
}

static int call_print_status(uint8_t argc, char * argv[])
{
	cmd_print_status();
	return 0;
}

static int call_flash_setting_reset(uint8_t argc, char * argv[])
{
	return cmd_flash_setting_reset();
}

#ifdef MODULE_STRIPLED
static int call_stripled_set_brightness(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "auto") == 0) {
		cmd_set_brightness_auto(true);
	} else {
		uint8_t brightness = strtol(argv[0], NULL, 10);
		if (brightness > 100) {
			term_print("Bad brightness level: " + String(brightness));
			return -1;
		}
		cmd_set_brightness(brightness);
	}
	return 0;
}

static int call_stripled_set_state(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "on") == 0) {
		return cmd_set_state(true);
	} else if (strcmp(argv[0], "off") == 0) {
		return cmd_set_state(false);
	} else if (strcmp(argv[0], "toggle") == 0) {
		return cmd_set_state(!cmd_get_state());
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
}

static int call_stripled_set_demo_mode(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "on") == 0) {
		return cmd_set_demo_mode(true);
	} else if (strcmp(argv[0], "off") == 0) {
		return cmd_set_demo_mode(false);
	} else if (strcmp(argv[0], "toggle") == 0) {
		return cmd_set_demo_mode(!cmd_get_demo_mode());
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
}

static int call_stripled_set_anim(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "off") == 0) {
		return cmd_set_animation(0);
	} else {
		uint8_t animID = strtol(argv[0], NULL, 10);
		return cmd_set_animation(animID);
	}
}
#endif

#ifdef MODULE_RELAY
static int call_relay_set_state(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "on") == 0) {
		return relay_set_state(true);
	} else if (strcmp(argv[0], "off") == 0) {
		return relay_set_state(false);
	} else if (strcmp(argv[0], "toggle") == 0) {
		return relay_set_state(!relay_get_state());
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
}
#endif

#ifdef MODULE_STATUS_LED
static int call_status_led_set_state(uint8_t argc, char * argv[])
{
	if (strcmp(argv[0], "on") == 0) {
		return cmd_set_status_led(true);
	} else if (strcmp(argv[0], "off") == 0) {
		return cmd_set_status_led(false);
	} else if (strcmp(argv[0], "toggle") == 0) {
		return cmd_set_status_led(!_isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED));
	} else {
		term_print("Unknown argument: " + String(argv[0]));
		return -1;
	}
}
#endif

/**
 * @brief Create the command line interface
 */
static int term_create_cli_commands(void)
{
	cli_token * tokRoot = cli_get_root_token();
	cli_token * tokLvl1;
	cli_token * tokLvl2;
	cli_token * curTok;

	// Create commands
	tokLvl1 = cli_add_token("board", "Board configuration");
	{
		curTok = cli_add_token("reset", "Reset module");
		cli_set_callback(curTok, &call_reset);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("status", "Print current status");
		cli_set_callback(curTok, &call_print_status);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);

	tokLvl1 = cli_add_token("flash", "Manage flash memory");
	{
		curTok = cli_add_token("default", "Reset flash setting to default");
		cli_set_callback(curTok, &call_flash_setting_reset);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);

#ifdef MODULE_STRIPLED
	tokLvl1 = cli_add_token("strip", "Manage stripled");
	{
		curTok = cli_add_token("brightness", "[level|auto] Set the brightness");
		cli_set_callback(curTok, &call_stripled_set_brightness);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("state", "[on|off|toggle] Set state of the stripled");
		cli_set_callback(curTok, &call_stripled_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("demo", "[on|off|toggle] Set demo mode");
		cli_set_callback(curTok, &call_stripled_set_demo_mode);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);

		curTok = cli_add_token("anim", "[animID|off] Set the animation ID");
		cli_set_callback(curTok, &call_stripled_set_anim);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

#ifdef MODULE_RELAY
	tokLvl1 = cli_add_token("relay", "Manage relay");
	{
		curTok = cli_add_token("set", "[on|off|toggle] Set the relay state");
		cli_set_callback(curTok, &call_relay_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

#ifdef MODULE_STATUS_LED
	tokLvl1 = cli_add_token("led", "Manage LED");
	{
		curTok = cli_add_token("set", "[on|off|toggle] Set the relay state");
		cli_set_callback(curTok, &call_status_led_set_state);
		cli_set_argc(curTok, 1, 0);
		cli_add_children(tokLvl1, curTok);
	}
	cli_add_children(tokRoot, tokLvl1);
#endif

	tokLvl1 = cli_add_token("lan", "LAN configuration");
	{
		curTok = cli_add_token("show", "[interface] Show configuration");
		cli_set_callback(curTok, &print_args);
		cli_set_argc(curTok, 0, 1);
		cli_add_children(tokLvl1, curTok);

		tokLvl2 = cli_add_token("set", "Define new configuration");
		{
			curTok = cli_add_token("ip", "<address> Set IP adress");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokLvl2, curTok);

			curTok = cli_add_token("gateway", "<address> Set gateway adress");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokLvl2, curTok);

			curTok = cli_add_token("mask", "<address> Set network mask");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokLvl2, curTok);

			curTok = cli_add_token("proxy", "<address> <port> Set the proxy location");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 2, 0);
			cli_add_children(tokLvl2, curTok);
		}
		cli_add_children(tokLvl1, tokLvl2);
	}
	cli_add_children(tokRoot, tokLvl1);
	return 0;
}

//========================
//	  PUBLIC FUNCTIONS
//========================

int term_init(void)
{
	cli_init();
	term_create_cli_commands();
	return 0;
}

void term_rx(uint8_t byte)
{
	cli_rx(byte);
}

void term_print(String str)
{
	uint32_t  len  = str.length();
	uint8_t * data = (uint8_t *) str.c_str();

#ifdef MODULE_SERIAL
	serial_write(data, len);
#endif
#ifdef MODULE_TELNET
	telnet_write(data, len);
#endif
}

#endif /* MODULE_TERM */