/**
  * @file   term.cpp
  * @brief  Terminal management
  * @author David DEVANT
  * @date   19/12/2020
  */

#include "global.hpp"
#include "term.hpp"
#include "cmd.hpp"
#include "serial.hpp"
#include "telnet.hpp"

#ifdef MODULE_TERM

struct term_t {
	char     rxBuffer[TERM_RX_BUFFER_SIZE];
	uint16_t rxLength;
	bool     isInCmdValid;
};
struct term_t termPort;

//========================
//	  STATIC FUNCTIONS
//========================

static void term_reset(void)
{
	termPort.isInCmdValid = false;
	termPort.rxLength = 0;
}

#define _term_ack(errorCode) term_ack(#errorCode, errorCode)
static void term_ack(const char errorCodeStr[], uint8_t errorCode)
{
	if (errorCode == OK) {
		term_print("CMD OK\n");
	} else {
		term_print("CMD ERROR: Code = " + String(errorCode) + " (" + errorCodeStr + ")\n");
	}
}

static void term_abort(void)
{
	term_print("\nABORT\n");
	term_reset();
}

static void term_execute_command(void)
{
	// If command is empty, display help menu
	if (termPort.rxLength == 0) {
		termPort.rxBuffer[0] = 'H';
		termPort.rxLength = 0;
	}

	// Get a new line for awnser
	switch (termPort.rxBuffer[0]) {
	case 'H':
		cmd_print_help();
		break;
	case 'T':
		cmd_print_status();
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

/**
 * @brief Do echo only on the interfaces
 * that don't have local echo
 */
static void term_echo(uint8_t byte)
{
#ifdef MODULE_SERIAL
	serial_write(byte);
#endif
}

static void term_tx(uint8_t byte)
{
#ifdef MODULE_SERIAL
	serial_write(byte);
#endif
#ifdef MODULE_TELNET
	telnet_write(byte);
#endif
}

//========================
//	  PUBLIC FUNCTIONS
//========================

void term_rx(uint8_t byte)
{
	char inChar;

	inChar = toupper(byte);

	// Transform all \r into \n (to support screen unix command)
	if (inChar == '\r') {
		inChar = '\n';
	}

	if (inChar == TERM_END_CMD) {
		termPort.isInCmdValid = true;
	} else if (inChar == TERM_ABORT_CMD) {
		term_abort();
	} else {
		if (termPort.rxLength < TERM_RX_BUFFER_SIZE) {
			termPort.rxBuffer[termPort.rxLength++] = inChar;
		} else {
			_term_ack(ERROR_BUFF_OVERRUN);
			term_abort();
		}
	}

	term_echo(inChar);
}

void term_print(String str)
{
	uint32_t len = str.length();

	// Send data one by one
	for (uint32_t i = 0; i < len; ++i) {
		term_tx(str[i]);
	}
}

void term_init(void)
{
	term_reset();
}

void term_main(void)
{
	if (termPort.isInCmdValid == true) {
		term_execute_command();
		term_reset();
	}
}

#endif /* MODULE_TERM */