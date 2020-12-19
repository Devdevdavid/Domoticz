#include "global.hpp"
#include "telnet.hpp"
#include "term.hpp"

#ifdef ESP32
#include <ESP32WebServer.h>
#else
#include <ESP8266WebServer.h>
#endif

#ifdef MODULE_TELNET

WiFiServer telnetServer(TELNET_PORT);
WiFiClient telnetClient;

void telnet_write(uint8_t byte)
{
	telnetClient.write(&byte, 1);
}

void telnet_init(void)
{
	telnetServer.begin();
 	telnetServer.setNoDelay(true);
}

void telnet_main(void)
{
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
    	term_rx(telnetClient.read());
  	}
}

#endif