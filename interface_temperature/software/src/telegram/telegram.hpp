#ifndef TELEGRAM_TELEGRAM_HPP
#define TELEGRAM_TELEGRAM_HPP

#include <Arduino.h>

#define TELEGRAM_CHECK_PERIOD_MS	1000 			/** Time between two check for new telegram messages */
#define TELEGRAM_DUMMY_MSG_COUNT	3				/** Number of dummy messages available */
// Shortcut
#if (TELEGRAM_LANG == TELEGRAM_LANG_EN)

#define TG_MSG_CHOOSE_OPTION					"Choisissez l'une des options suivantes"
#define TG_MSG_CONNECTION_OK					"La connexion est OK"
#define TG_MSG_TEMPERATURE_IS					"La température est de "
#define TG_MSG_UNKNOWN_CMD						"Cette commande n'est pas supportée"
#define TG_MSG_IP_ADDRESS						"Adresse IP : "
#define TG_MSG_BAD_RELAY_FEEDBACK				"Le relais semble défectueux !"
#define TG_MSG_HAD_BEEN_STARTED					"System is now started"
#define TG_MSG_HAD_BEEN_STOPPED					"System is now stopped"

#define TG_MSG_DUMMY_1							"Sure !"
#define TG_MSG_DUMMY_2							"What ever you wish master !"
#define TG_MSG_DUMMY_3							"No ? Really ?"

#elif (TELEGRAM_LANG == TELEGRAM_LANG_FR)

#define TG_MSG_CHOOSE_OPTION					"Choose one of the following options"
#define TG_MSG_CONNECTION_OK					"Connection is OK"
#define TG_MSG_TEMPERATURE_IS					"Temperature is "
#define TG_MSG_UNKNOWN_CMD						"This command is not supported"
#define TG_MSG_IP_ADDRESS						"IP Address: "
#define TG_MSG_BAD_RELAY_FEEDBACK				"Relay seems faulty !"
#define TG_MSG_HAD_BEEN_STARTED					"Le système est désormais en marche"
#define TG_MSG_HAD_BEEN_STOPPED					"Le système est désormais arrété"

#define TG_MSG_DUMMY_1							"Bien sur !"
#define TG_MSG_DUMMY_2							"Ce que vous voudrez maître !"
#define TG_MSG_DUMMY_3							"Non ? Vraiment ?"

#endif


void telegram_init(void);
void telegram_main(void);
void telegram_send_msg_temperature(uint8_t sensorID, float degreesValue);

// Externs
#ifndef TELEGRAM_TELEGRAM_CPP
extern bool isAutoTempMsgEnabled;
#endif


#endif /* TELEGRAM_TELEGRAM_HPP */
