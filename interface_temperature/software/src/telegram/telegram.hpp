#ifndef TELEGRAM_TELEGRAM_HPP
#define TELEGRAM_TELEGRAM_HPP

#include <Arduino.h>

void telegram_send_temperature(uint8_t sensorID, float degreesValue);
void telegram_init(void);

#endif /* TELEGRAM_TELEGRAM_HPP */
