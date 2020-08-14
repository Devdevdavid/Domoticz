#ifndef TELEGRAM_TELEGRAM_HPP
#define TELEGRAM_TELEGRAM_HPP

#include <Arduino.h>

#define TELEGRAM_CHECK_PERIOD_MS	5*1000 			/** Time between two check for new telegram messages (Should not be under 2.8s because telegram update takes time) */
#define TELEGRAM_DUMMY_MSG_COUNT	3				/** Number of dummy messages available */

// EMOJIS
// https://apps.timwhitlock.info/emoji/tables/unicode
#define EMOJI_ROCKET 				"\xF0\x9F\x9A\x80"
#define EMOJI_GREEN_CHECK 			"\xE2\x9C\x85"
#define EMOJI_CROSS_MARK 			"\xE2\x9D\x8C"
#define EMOJI_RED_REVOLVING_LIGHT 	"\xF0\x9F\x9A\xA8"
#define EMOJI_NUMBER_SIGN 			"\x23\xE2\x83\xA3"
#define EMOJI_QUESTION_MARK 		"\xE2\x9D\x93"
#define EMOJI_INFORMATION_MARK 		"\xE2\x84\xB9"

// Traducted messages
#if (G_LANG == G_LANG_FR)

	#define TG_MSG_CHOOSE_OPTION					"Choisissez l'une des options suivantes"
	#define TG_MSG_CONNECTION_OK					"La connexion est OK"
	#define TG_MSG_TEMPERATURE_IS					"La température est de "
	#define TG_MSG_UNKNOWN_CMD						"Cette commande n'est pas supportée"
	#define TG_MSG_IP_ADDRESS						"Adresse IP : "
	#define TG_MSG_BAD_RELAY_FEEDBACK				"Le relais semble défectueux !"
	#define TG_MSG_BAD_TEMP_SENSOR					"Le capteur est défectueux"
	#define TG_MSG_UNUSED_TEMP_SENSOR				"Capteur non utilisé"
	#define TG_MSG_HAD_BEEN_STARTED					"Le système est désormais en marche"
	#define TG_MSG_HAD_BEEN_STOPPED					"Le système est désormais arrété"
	#define TG_MSG_ALERT_GOES_ON					"L'ALERTE EST DÉSORMAIS ACTIVE"
	#define TG_MSG_ALERT_GOES_OFF					"L'alerte est désormais inactive"
	#define TG_MSG_ALERT_IS_ON						"L'alerte est active"
	#define TG_MSG_OPT_GOES_ON						"L'interrupteur d'option est actif"
	#define TG_MSG_OPT_GOES_OFF						"L'interrupteur d'option est inactif"
	#define TG_MSG_ALERT_METHOD_THRESHOLD			"Les capteurs sont en mode SEUIL"
	#define TG_MSG_ALERT_METHOD_DIFFERENTIAL		"Les capteurs sont en mode DIFFERENTIEL (T1 - T0 >= "
	#define TG_MSG_SENSOR_THRESHOLD					"Seuil configuré"

	#define TG_MSG_DUMMY_1							"Bien sur !"
	#define TG_MSG_DUMMY_2							"Ce que vous voudrez maître !"
	#define TG_MSG_DUMMY_3							"Non ? Vraiment ?"

#elif (G_LANG == G_LANG_EN)

	#define TG_MSG_CHOOSE_OPTION					"Choose one of the following options"
	#define TG_MSG_CONNECTION_OK					"Connection is OK"
	#define TG_MSG_TEMPERATURE_IS					"Temperature is "
	#define TG_MSG_UNKNOWN_CMD						"This command is not supported"
	#define TG_MSG_IP_ADDRESS						"IP Address: "
	#define TG_MSG_BAD_RELAY_FEEDBACK				"Relay seems faulty !"
	#define TG_MSG_BAD_TEMP_SENSOR					"Sensor is faulty"
	#define TG_MSG_UNUSED_TEMP_SENSOR				"Sensor is unused"
	#define TG_MSG_HAD_BEEN_STARTED					"System is now started"
	#define TG_MSG_HAD_BEEN_STOPPED					"System is now stopped"
	#define TG_MSG_ALERT_GOES_ON					"ALERT IS NOW ON"
	#define TG_MSG_ALERT_GOES_OFF					"Alert is now off"
	#define TG_MSG_ALERT_IS_ON						"Alert is ON"
	#define TG_MSG_OPT_GOES_ON						"Option switch is enabled"
	#define TG_MSG_OPT_GOES_OFF						"Option switch is disabled"
	#define TG_MSG_ALERT_METHOD_THRESHOLD			"Sensors are in THRESHOLD mode"
	#define TG_MSG_ALERT_METHOD_DIFFERENTIAL		"Sensors are in DIFFERENTIAL mode (T1 - T0 >= "
	#define TG_MSG_SENSOR_THRESHOLD					"Configured threshold"

	#define TG_MSG_DUMMY_1							"Sure !"
	#define TG_MSG_DUMMY_2							"What ever you wish master !"
	#define TG_MSG_DUMMY_3							"No ? Really ?"

#endif


void telegram_init(void);
void telegram_main(void);
void telegram_send_msg_temperature(uint8_t sensorID, float degreesValue);
void telegram_send_alert(bool isInAlert);
void telegram_send_opt_changed(bool isOptEnabled);
void telegram_send_conn_ok(void);

// Externs
#ifndef TELEGRAM_TELEGRAM_CPP
extern bool isAutoTempMsgEnabled;
#endif


#endif /* TELEGRAM_TELEGRAM_HPP */
