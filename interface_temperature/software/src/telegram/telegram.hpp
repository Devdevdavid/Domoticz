#ifndef TELEGRAM_TELEGRAM_HPP
#define TELEGRAM_TELEGRAM_HPP

#include <Arduino.h>

#define TELEGRAM_CHECK_PERIOD_MS	1000 			/** Time between two check for new telegram messages */

// Shortcut
#if (TELEGRAM_LANG == TELEGRAM_LANG_EN)

#define TG_MSG_CHOOSE_OPTION					"Choisissez l'une des options suivantes"
#define TG_MSG_CONNECTION_OK					"La connexion est OK"
#define TG_MSG_TEMPERATURE_IS					"La température est de "

#elif (TELEGRAM_LANG == TELEGRAM_LANG_FR)

#define TG_MSG_CHOOSE_OPTION					"Choose one of the following options"
#define TG_MSG_CONNECTION_OK					"Connection is OK"
#define TG_MSG_TEMPERATURE_IS					"Temperature is "

#endif


void telegram_init(void);
void telegram_main(void);
void telegram_send_msg_temperature(uint8_t sensorID, float degreesValue);



#endif /* TELEGRAM_TELEGRAM_HPP */
