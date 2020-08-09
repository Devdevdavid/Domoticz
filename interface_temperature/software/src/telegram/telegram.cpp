#include <stdlib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#define TELEGRAM_TELEGRAM_CPP

#include "global.hpp"
#include "telegram.hpp"
#include "temp/temp.hpp"

#ifdef MODULE_TELEGRAM

// EXTERNS
extern uint32_t tick;

// VARIABLES
bool isAutoTempMsgEnabled = false;

// STATIC
static uint32_t nextCheckTick = 0;
static String linkedChat;
static WiFiClientSecure wiFiClientSecure;
static UniversalTelegramBot TBot(TELEGRAM_CONV_TOKEN, wiFiClientSecure);
static String dummyMessages[TELEGRAM_DUMMY_MSG_COUNT] = {
	TG_MSG_DUMMY_1,
	TG_MSG_DUMMY_2,
	TG_MSG_DUMMY_3
};
static char dummyBytes[3 * 8 + 1];
static String reply = "";
static const String keyboardJson = "[[\"/start\", \"/stop\"], [\"/status\", \"/sensors\"]]";

/**
 * @brief Send a reply to the linked chat
 * with a special inline keyboard
 *
 * @param msg The string to send
 */
static void telegram_send(String msg)
{
	// We have nowhere to send this message
	if (linkedChat == "") {
		return;
	}

	TBot.sendMessageWithReplyKeyboard(linkedChat, msg, "Markdown", keyboardJson);
}

/**
 * @brief Send a brief message to explain all available commands
 */
static String telegram_append_motd(String msg)
{
	msg += FIRMWARE_VERSION "\n\n";

#if (G_LANG == G_LANG_FR)
	msg += "/start : Démarre l'annonce de la température sur Telegram\n";
	msg += "/stop : Arrête l'annonce de la température sur Telegram\n";
	msg += "/sensors : Affiche les adresses des capteurs de température\n";
	msg += "/status : Retourne le statut du système\n";
#elif (G_LANG == G_LANG_EN)
	msg += "/start:  Starting temperature announcement on telegram\n";
	msg += "/stop: Stopping temperature announcement on telegram\n";
	msg += "/sensors: Show temperature sensor addresses\n";
	msg += "/status: Return the status of the system\n";
#endif

	return msg;
}

/**
 * @brief Execute commands received from telegram
 *
 * @param message The message object given by Telegram API
 */
static void telegram_handle_new_message(telegramMessage * message) {
	// Save the chat to wich the reply must be send
	linkedChat = String(message->chat_id);

	if (message->text == "/start") {
		isAutoTempMsgEnabled = true;

		reply = EMOJI_ROCKET " " TG_MSG_HAD_BEEN_STARTED "\n";
		reply = telegram_append_motd(reply);
		telegram_send(reply);
	}
	else if (message->text == "/stop") {
		isAutoTempMsgEnabled = false;
		telegram_send(EMOJI_CROSS_MARK " " TG_MSG_HAD_BEEN_STOPPED);
	}
	else if (message->text == "/status") {
		reply += FIRMWARE_VERSION "\n";
		reply += EMOJI_NUMBER_SIGN " " TG_MSG_IP_ADDRESS + WiFi.localIP().toString() + "\n";

		// Is the script in alert ?
		if (_isset(STATUS_SCRIPT, STATUS_SCRIPT_IN_ALERT)) {
			reply += EMOJI_RED_REVOLVING_LIGHT " " TG_MSG_ALERT_IS_ON "\n";
		}

#ifdef MODULE_RELAY
		// Specialized error messages
		if (_isset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT)) {
			reply += EMOJI_CROSS_MARK " " TG_MSG_BAD_RELAY_FEEDBACK "\n";
		}
#endif
#ifdef MODULE_TEMPERATURE
		// Check sensors
		for (int i = 0; i < TEMP_MAX_SENSOR_SUPPORTED; ++i) {
			reply += "`Temp. " + String(i) + "] `";

			// Is the sensor used ?
			if (i >= temp_get_nb_sensor()) {
				reply += TG_MSG_UNUSED_TEMP_SENSOR "\n";
			} else {
				// Test the state of the sensor
				if (_isset(STATUS_TEMP, STATUS_TEMP_1_FAULT << i)) {
					reply += EMOJI_CROSS_MARK " " TG_MSG_BAD_TEMP_SENSOR "\n";
				} else {
					reply += EMOJI_GREEN_CHECK " " + String(temp_get_value(i)) + " 'C\n";
				}
			}
		}
#endif

		telegram_send(reply);
	}
#ifdef MODULE_TEMPERATURE
	else if (message->text == "/sensors") {
		// Indicate the mode used
#if (SCRIPT_TEMP_ALERT_METHOD == METHOD_THRESHOLD)
		reply += TG_MSG_ALERT_METHOD_THRESHOLD "\n";
#elif (SCRIPT_TEMP_ALERT_METHOD == METHOD_DIFFERENTIAL)
		reply += TG_MSG_ALERT_METHOD_DIFFERENTIAL "\n";
#endif
		// Display addresses
		for (int i = 0; i < TEMP_MAX_SENSOR_SUPPORTED; ++i) {
			reply += "`Temp. " + String(i) + "] `";

			// Is the sensor used ?
			if (i >= temp_get_nb_sensor()) {
				reply += TG_MSG_UNUSED_TEMP_SENSOR "\n";
			} else {
				reply += String(temp_get_address(dummyBytes, i)) + "\n";
			}
		}

		telegram_send(reply);
#endif
	} else if (message->text[0] == '/') {
		// Command not supported
		reply = EMOJI_QUESTION_MARK " " TG_MSG_UNKNOWN_CMD "\n\n";
		reply = telegram_append_motd(reply);
		telegram_send(reply);
	} else {
		// Init rand()
		srand(tick);

		// Send a dummy message when message is not a command
		uint8_t randomInt = rand() % TELEGRAM_DUMMY_MSG_COUNT;
		telegram_send(dummyMessages[randomInt]);
	}
}
// =====================
// FUNCTIONS
// =====================

/**
 * @brief Inititalize the telegram module
 */
void telegram_init(void)
{
	nextCheckTick = 0;

	// This is the simplest way of getting this working
  	// if you are passing sensitive information, or controlling
  	// something important, please either use certStore or at
  	// least client.setFingerPrint
	// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/104#issuecomment-485255312
	wiFiClientSecure.setInsecure();
}

/**
 * @brief Main function of the telegram module
 */
void telegram_main(void)
{
	int msgNumber;

	if (tick >= nextCheckTick)  {
		nextCheckTick = tick + TELEGRAM_CHECK_PERIOD_MS;

		do {
			// This take a loooot of time (2.8s)
			msgNumber = TBot.getUpdates(TBot.last_message_received + 1);

			// For each messages
			for (int i = 0; i < msgNumber; i++) {
				log_info("TBot says: %s", TBot.messages[i].text.c_str());
				telegram_handle_new_message(&TBot.messages[i]);
			}
		} while (msgNumber);
	}
}

/**
 * @brief Function called by the module Script to periodically send messages
 * on the current temperature
 *
 * @param sensorID [0-1] The index of the sensor
 * @param degreesValue The value of the sensor in degrees
 */
void telegram_send_msg_temperature(uint8_t sensorID, float degreesValue)
{
	String msg = String(sensorID) + "] " + TG_MSG_TEMPERATURE_IS + String(degreesValue) + "'C";
	telegram_send(msg);
}

/**
 * @brief Tell the client that alert changed
 *
 * @param isInAlert boolean
 */
void telegram_send_alert(bool isInAlert)
{
	if (isInAlert) {
		telegram_send(EMOJI_RED_REVOLVING_LIGHT " " TG_MSG_ALERT_GOES_ON " " EMOJI_RED_REVOLVING_LIGHT);
	} else {
		telegram_send(EMOJI_GREEN_CHECK " " TG_MSG_ALERT_GOES_OFF);
	}
}

/**
 * @brief Tell the client that OPT switch changed
 *
 * @param isOptEnabled boolean
 */
void telegram_send_opt_changed(bool isOptEnabled)
{
	if (isOptEnabled) {
		telegram_send(EMOJI_INFORMATION_MARK " " TG_MSG_OPT_GOES_ON);
	} else {
		telegram_send(EMOJI_INFORMATION_MARK " " TG_MSG_OPT_GOES_OFF);
	}
}

/**
 * @brief Tell the client that connexion is OK
 */
void telegram_send_conn_ok(void)
{
	telegram_send(EMOJI_GREEN_CHECK " " TG_MSG_CONNECTION_OK);
}

#endif