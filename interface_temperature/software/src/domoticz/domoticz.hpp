#ifndef DOMOTICZ_DOMOTICZ_HPP
#define DOMOTICZ_DOMOTICZ_HPP

#include <Arduino.h>

void domoticz_send_temperature(uint8_t sensorID, float degreesValue);
void domoticz_init(void);

#endif /* DOMOTICZ_DOMOTICZ_HPP */
