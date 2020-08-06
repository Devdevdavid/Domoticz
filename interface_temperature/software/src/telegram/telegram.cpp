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

/**
 * @brief Send a brief message to explain all available commands
 */
static void telegram_msg_send_motd(void)
{
#if (G_LANG == G_LANG_FR)
	String welcome = "Interface ES ESP32 alarme.\n";
	welcome += "`/start  `: Mise en marche central d'alarme\n";
	welcome += "`/stop   `: Arrêt central d'alarme\n";
	welcome += "`/status `: Retourne le statut de la connexion\n";
#elif (G_LANG == G_LANG_EN)
	String welcome = "Interface ES ESP32 alarm.\n";
	welcome += "`/start  `: Starting alarm station\n";
	welcome += "`/stop   `: Shutdown alarm station\n";
	welcome += "`/status `: Return connection state\n";
#endif
	TBot.sendMessage(linkedChat, welcome, "Markdown"); // mise en forme du texte
}

/**
 * @brief Execute commands received from telegram
 *
 * @param message The message object given by Telegram API
 */
static void telegram_handle_new_message(telegramMessage * message) {
	String reply = "";
	String keyboardJson = "[[\"/start\", \"/stop\"], [\"/status\"]]";

	// Save the chat to wich the reply must be send
	linkedChat = String(message->chat_id);

	if (message->text == "/start") {
		isAutoTempMsgEnabled = true;

		TBot.sendMessageWithReplyKeyboard(linkedChat, EMOJI_ROCKET " " TG_MSG_HAD_BEEN_STARTED, "", keyboardJson, true);
	}
	else if (message->text == "/stop") {
		isAutoTempMsgEnabled = false;
		TBot.sendMessage(linkedChat, EMOJI_CROSS_MARK " " TG_MSG_HAD_BEEN_STOPPED, "");
	}
	else if (message->text == "/status") {
		reply += FIRMWARE_VERSION "\n";
		reply += EMOJI_NUMBER_SIGN " " TG_MSG_IP_ADDRESS + WiFi.localIP().toString() + "\n";

#ifdef MODULE_RELAY
		// Specialized error messages
		if (_isset(STATUS_APPLI, STATUS_APPLI_RELAY_FAULT)) {
			reply += EMOJI_CROSS_MARK " " TG_MSG_BAD_RELAY_FEEDBACK"\n";
		}
#endif
#ifdef MODULE_TEMPERATURE
		// Check sensors
		for (int i = 0; i < TEMP_MAX_SENSOR_SUPPORTED; ++i)
		{
			reply += "`Temp. " + String(i) + "] `";
			if (_isset(STATUS_APPLI, STATUS_APPLI_TEMP_1_FAULT << i)) {
				reply += EMOJI_CROSS_MARK " " TG_MSG_BAD_TEMP_SENSOR"\n";
			} else {
				reply += EMOJI_GREEN_CHECK " " + String(temp_get_value(i)) + " 'C\n";
			}
		}
#endif

		TBot.sendMessage(linkedChat, reply, "Markdown");
	}
	else if (message->text[0] == '/') {
		// Command not supported
		TBot.sendMessage(linkedChat, EMOJI_QUESTION_MARK " " TG_MSG_UNKNOWN_CMD, "");
		telegram_msg_send_motd();
	} else {
		// Init rand()
		srand(tick);

		// Send a dummy message when message is not a command
		uint8_t randomInt = rand() % TELEGRAM_DUMMY_MSG_COUNT;
		TBot.sendMessage(linkedChat, dummyMessages[randomInt], "");
	}
}

// FUNCTION

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
		log_info("TBot Update %d", tick);

		do {
			msgNumber = TBot.getUpdates(TBot.last_message_received + 1);
			for (int i = 0; i < msgNumber; i++) {
				log_info("Handling TBot message %d: %s", i, TBot.messages[i].text.c_str());
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
	// We have nowhere to send this message
	if (linkedChat == "") {
		return;
	}

	String msg = String(sensorID) + "] " + TG_MSG_TEMPERATURE_IS + String(degreesValue) + "'C";
	TBot.sendMessage(linkedChat, msg, "");
	log_info("Sending \"%s\" to %s", msg.c_str(), linkedChat.c_str());
}

/**
 * @brief Tell the client that alert changed
 *
 * @param isInAlert boolean
 */
void telegram_send_alert(bool isInAlert)
{
	// We have nowhere to send this message
	if (linkedChat == "") {
		return;
	}

	if (isInAlert) {
		TBot.sendMessage(linkedChat, EMOJI_RED_REVOLVING_LIGHT " " TG_MSG_ALERT_GOES_ON " " EMOJI_RED_REVOLVING_LIGHT, "");
	} else {
		TBot.sendMessage(linkedChat, EMOJI_GREEN_CHECK " " TG_MSG_ALERT_GOES_OFF, "");
	}
}

#endif