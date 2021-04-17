/**
  * @file   telnet.hpp
  * @brief  Manage Telnet interface
  * @author David DEVANT
  * @date   16/12/2020
  */

#ifndef TELNET_H
#define TELNET_H

#include <Arduino.h>

#define TELNET_PORT 23

void telnet_write(uint8_t * data, uint8_t len);
int  telnet_init(void);
void telnet_main(void);

#endif /* TELNET_H */
