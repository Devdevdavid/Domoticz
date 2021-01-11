/**
  * @file   serial.cpp
  * @brief  Manage serial interface
  * @author David DEVANT
  * @date   12/08/2018
  */

#include "global.hpp"
#include "serial.hpp"
#include "term.hpp"

#ifdef MODULE_SERIAL

void serial_write(uint8_t byte)
{
	Serial.write(byte);
	if (byte == '\n') {
		Serial.write('\r');
	}
}

void serial_init(void)
{
	Serial.begin(115200);
	Serial.print("\n\r\n\r"); // Jump some lines after internal firmware stuff
}

void serial_main(void)
{
	// Send all we received to the terminal
	while (Serial.available() > 0) {
		term_rx(Serial.read());
	}
}

#endif /* MODULE_SERIAL */
