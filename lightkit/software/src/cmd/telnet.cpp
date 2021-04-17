#include "telnet.hpp"
#include "global.hpp"
#include "term.hpp"

#ifdef ESP32
#include <ESP32WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif

#ifdef MODULE_TELNET

/** Used to manage escaping sequence state machine */
typedef enum
{
	ESC_IDDLE = 0,       /**< Wait for 0xFF to come in */
	ESC_WAIT_COMMAND,    /**< Wait for the telnet command to follow */
	ESC_WAIT_SUB_COMMAND /**< Wait for the telnet sub-command to follow */
} escape_state_e;

// Internal variables
WiFiServer     telnetServer(TELNET_PORT);
WiFiClient     telnetClient;
escape_state_e escapingState;

// ==================
//  FUNCTIONS
// ==================

void telnet_write(uint8_t * data, uint8_t len)
{
	if (telnetClient.connected()) {
		telnetClient.write(data, len);
	}
}

int telnet_init(void)
{
	escapingState = ESC_IDDLE;

	telnetServer.begin();
	telnetServer.setNoDelay(true);
	return 0;
}

void telnet_main(void)
{
	uint8_t byte;

	// Take the waiting client as active if none is currently handled
	if (telnetServer.hasClient()) {
		if (!telnetClient || !telnetClient.connected()) {
			if (telnetClient) {
				telnetClient.stop();
			}
			telnetClient = telnetServer.available();
			telnetClient.flush();
		}
	}

	// Read Data from client and send it to terminal
	while (telnetClient.available()) {
		byte = telnetClient.read();

		// Filter out telnet commands
		switch (escapingState) {
		case ESC_IDDLE:
			if (byte == 0xFF) {
				escapingState = ESC_WAIT_COMMAND;
				continue;
			}
			break;
		case ESC_WAIT_COMMAND:
			// See TELNET COMMAND STRUCTURE from RFC 854
			if ((byte >= 0xF0) && (byte < 0xFF)) {
				escapingState = ESC_WAIT_SUB_COMMAND;
				continue;
			} else {
				// Was not an escape sequence, oups, manage byte normally
				escapingState = ESC_IDDLE;
			}
			break;
		case ESC_WAIT_SUB_COMMAND:
		default:
			// Ignore params
			escapingState = ESC_IDDLE;
			continue;
		}

		// Manage the byte if not an escape sequence
		term_rx(byte);
	}
}

#endif