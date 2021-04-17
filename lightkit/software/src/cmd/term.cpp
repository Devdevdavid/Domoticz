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

/**
 * @brief Create the command line interface
 */
static int term_create_cli_commands(void)
{
	cli_token * tokRoot = cli_get_root_token();
	cli_token * tokLan;
	cli_token * tokIpSet;
	cli_token * curTok;

	// Create commands
	curTok = cli_add_token("exit", "Exit application");
	cli_set_callback(curTok, &print_args);
	cli_add_children(tokRoot, curTok);

	tokLan = cli_add_token("lan", "LAN configuration");
	{
		curTok = cli_add_token("show", "[interface] Show configuration");
		cli_set_callback(curTok, &print_args);
		cli_set_argc(curTok, 0, 1);
		cli_add_children(tokLan, curTok);

		tokIpSet = cli_add_token("set", "Define new configuration");
		{
			curTok = cli_add_token("ip", "<address> Set IP adress");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokIpSet, curTok);

			curTok = cli_add_token("gateway", "<address> Set gateway adress");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokIpSet, curTok);

			curTok = cli_add_token("mask", "<address> Set network mask");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 1, 0);
			cli_add_children(tokIpSet, curTok);

			curTok =
			cli_add_token("proxy", "<address> <port> Set the proxy location");
			cli_set_callback(curTok, &print_args);
			cli_set_argc(curTok, 2, 0);
			cli_add_children(tokIpSet, curTok);
		}
		cli_add_children(tokLan, tokIpSet);
	}
	cli_add_children(tokRoot, tokLan);
	return 0;
}

#ifdef DISABLEDNONO
static void term_execute_command(void)
{
	// If command is empty, display help menu
	if (termPort.rxLength == 0) {
		termPort.rxBuffer[0] = 'H';
		termPort.rxLength    = 0;
	}

	// Get a new line for awnser
	switch (termPort.rxBuffer[0]) {
	case 'H':
		cmd_print_help();
		break;
	case 'I':
		cmd_reset_module();
		break;
	case 'T':
		cmd_print_status();
		break;
	case 'F':
		if (cmd_flash_setting_reset()) {
			_term_ack(ERROR_INTERNAL);
		} else {
			_term_ack(OK);
		}
		break;
#ifdef MODULE_STATUS_LED
	case 'L':
		if (termPort.rxLength == 1) {
			cmd_set_status_led(!_isset(STATUS_APPLI, STATUS_APPLI_STATUS_LED));
		} else if (termPort.rxLength == 2) {
			cmd_set_status_led(sToU16(&termPort.rxBuffer[1], 1));
		} else {
			_term_ack(ERROR_WRONG_LENGTH);
			break;
		}
		_term_ack(OK);
		break;
#endif
#ifdef MODULE_STRIPLED
	case 'B':
		if (termPort.rxLength < 1) {
			_term_ack(ERROR_WRONG_LENGTH);
		} else if (termPort.rxLength == 1) {
			cmd_set_brightness_auto(true);
			_term_ack(OK);
		} else {
			uint8_t brightness = sToU16(&termPort.rxBuffer[1], termPort.rxLength - 1);
			if (brightness <= 100) {
				cmd_set_brightness(brightness);
				_term_ack(OK);
			} else {
				_term_ack(ERROR_WRONG_VALUE);
			}
		}
		break;
#endif
#ifdef MODULE_STRIPLED
	/** SWITCH: ON/OFF */
	case 'S':
		if (termPort.rxLength < 1) {
			_term_ack(ERROR_WRONG_LENGTH);
		} else if (termPort.rxLength == 1) {
			/** TOGGLE STATE */
			cmd_set_state(!cmd_get_state());
			_term_ack(OK);
		} else {
			if (termPort.rxBuffer[1] == '1') {
				/** SET to ON */
				cmd_set_state(true);
				_term_ack(OK);
			} else if (termPort.rxBuffer[1] == '0') {
				/** SET to OFF */
				cmd_set_state(false);
				_term_ack(OK);
			} else {
				_term_ack(ERROR_WRONG_VALUE);
			}
		}
		break;
#endif
#ifdef MODULE_STRIPLED
	/** DEMO MODE: ON/OFF */
	case 'D':
		if (termPort.rxLength < 1) {
			_term_ack(ERROR_WRONG_LENGTH);
		} else if (termPort.rxLength == 1) {
			/** TOGGLE STATE */
			cmd_set_demo_mode(!cmd_get_demo_mode());
			_term_ack(OK);
		} else {
			if (termPort.rxBuffer[1] == '1') {
				/** SET to ON */
				cmd_set_demo_mode(true);
				_term_ack(OK);
			} else if (termPort.rxBuffer[1] == '0') {
				/** SET to OFF */
				cmd_set_demo_mode(false);
				_term_ack(OK);
			} else {
				_term_ack(ERROR_WRONG_VALUE);
			}
		}
		break;
#endif
#ifdef MODULE_STRIPLED
	/** ANIMATION */
	case 'A':
		if (termPort.rxLength <= 1) {
			_term_ack(ERROR_WRONG_LENGTH);
		} else {
			uint16_t animID = sToU16(&termPort.rxBuffer[1], termPort.rxLength - 1);
			if (cmd_set_animation(animID) == OK) {
				_term_ack(OK);
			} else {
				_term_ack(ERROR_WRONG_VALUE);
			}
		}
		break;
#endif
#ifdef MODULE_RELAY
	case 'R':
		if (termPort.rxLength == 1) {
			relay_set_state(!relay_get_state());
		} else if (termPort.rxLength == 2) {
			relay_set_state(sToU16(&termPort.rxBuffer[1], 1));
		} else {
			_term_ack(ERROR_WRONG_LENGTH);
			break;
		}
		_term_ack(OK);
		break;
#endif
	default:
		_term_ack(ERROR_UNKNOWN_CMD);
		break;
	}
}
#endif

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