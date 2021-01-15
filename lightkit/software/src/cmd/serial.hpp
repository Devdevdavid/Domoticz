/**
  * @file   serial.hpp
  * @brief  Manage serial interface
  * @author David DEVANT
  * @date   12/08/2018
  */

#ifndef CMD_SERIAL_H
#define CMD_SERIAL_H

#include <Arduino.h>

void serial_write(uint8_t byte);
int  serial_init(void);
void serial_main(void);

#endif /* CMD_SERIAL_H */
