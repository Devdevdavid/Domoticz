/**
  * @file   domoticz.hpp
  * @brief  Handle domoticz server communication
  * @author David DEVANT
  * @date   12/08/2019
  */

#ifndef DOMOTICZ_DOMOTICZ_HPP
#define DOMOTICZ_DOMOTICZ_HPP

#include <Arduino.h>

void domoticz_send_temperature(uint8_t sensorID, float degreesValue);
void domoticz_init(void);

#endif /* DOMOTICZ_DOMOTICZ_HPP */
