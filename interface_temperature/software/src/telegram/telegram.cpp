/**
  * @file   telegram.cpp
  * @brief  Manage communication with Telegram servers
  * @author David DEVANT
  * @date   12/08/2020
  */

#include <stdlib.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

#include "global.hpp"
#include "telegram.hpp"
#include "temp/temp.hpp"
#include "io/inputs.hpp"
#include "relay/relay.hpp"

#ifdef MODULE_TELEGRAM

// STRUCT
struct telegram_cmd_t {
	char text[TG_CMD_TEXT_LEN];
	char desc[TG_CMD_DESC_LEN];
	String (*callback)(String);
};

// EXTERNS
extern uint32_t tick;
extern float sensorThreshold[TEMP_MAX_SENSOR_SUPPORTED];

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
static String keyboardJson = "";

static struct telegram_cmd_t telegramCmds[TG_CMD_MAX];
static uint32_t telegramCmdsCount;

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

	for (uint8_t i = 0; i < telegramCmdsCount; ++i) {
		msg += String(telegramCmds[i].text);
		msg += ": ";
		msg += String(telegramCmds[i].desc);
		msg += "\n";
	}

	return msg;
}

/**
 * @brief Execute commands received from telegram
 *
 * @param message The message object given by Telegram API
 */
static void telegram_handle_new_message(telegramMessage * message) {
	uint8_t i;

	// Reset reply
	reply = "";

	// Save the chat to wich the reply must be send
	linkedChat = String(message->chat_id);

	// Search for a known command
	for (i = 0; i < telegramCmdsCount; ++i) {
		if (strncmp(telegramCmds[i].text, message->text.c_str(), TG_CMD_TEXT_LEN) == 0) {
			reply = telegramCmds[i].callback(reply);
			break;
		}
	}

	// Command not found in telegramCmds ?
	if (i == telegramCmdsCount) {
		if (message->text[0] == '/') {
			// Command not supported
			reply = EMOJI_QUESTION_MARK " " TG_MSG_UNKNOWN_CMD "\n\n";
			reply = telegram_append_motd(reply);
		} else {
			// Init rand()
			srand(tick);

			// Send a dummy message when message is not a command
			uint8_t randomInt = rand() % TELEGRAM_DUMMY_MSG_COUNT;
			reply = dummyMessages[randomInt];
		}
	}

	telegram_send(reply);
}

// =====================
// COMMAND CALLBACKS
// =====================

static String telegram_cmd_status(String reply)
{
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
	} else {
		reply += EMOJI_GREEN_CHECK " " TG_MSG_GOOD_RELAY_FEEDBACK "\n";
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

	return reply;
}

static String telegram_cmd_start(String reply)
{
	isAutoTempMsgEnabled = true;

	reply = EMOJI_ROCKET " " TG_MSG_HAD_BEEN_STARTED "\n\n";
	reply = telegram_append_motd(reply);

	return reply;
}

static String telegram_cmd_stop(String reply)
{
	isAutoTempMsgEnabled = false;

	reply += EMOJI_CROSS_MARK " " TG_MSG_HAD_BEEN_STOPPED;
	return reply;
}

#if defined(MODULE_TEMPERATURE)
static String telegram_cmd_sensors(String reply)
{
	// Indicate the mode used
#if (SCRIPT_TEMP_ALERT_METHOD == METHOD_THRESHOLD)
	reply += TG_MSG_ALERT_METHOD_THRESHOLD "\n";
#elif (SCRIPT_TEMP_ALERT_METHOD == METHOD_DIFFERENTIAL)
	reply += TG_MSG_ALERT_METHOD_DIFFERENTIAL + String(SCRIPT_TEMP_ALERT_DIFF_THRESHOLD) + "°C)\n";
#endif
	// Display addresses
	for (int i = 0; i < TEMP_MAX_SENSOR_SUPPORTED; ++i) {
		reply += "`Temp. " + String(i) + "] `";

		// Is the sensor used ?
		if (i >= temp_get_nb_sensor()) {
			reply += TG_MSG_UNUSED_TEMP_SENSOR;
		} else {
			reply += String(temp_get_address(dummyBytes, i));
		}

		// Display the configure threshold
#if (SCRIPT_TEMP_ALERT_METHOD == METHOD_THRESHOLD)
		reply += " (" TG_MSG_SENSOR_THRESHOLD ": " + String(sensorThreshold[i]) + "°C)";
#endif
		// Add ending line
		reply += "\n";
	}

	return reply;
}
#endif

#if defined(MODULE_RELAY)
static String telegram_cmd_relay(String reply)
{
	/** Force alarm off if OPT is enabled
      *  Switch is set when signal is LOW */
	if (is_input_high(INPUTS_OPT_TEMP_ALARM_EN)) {
		reply += EMOJI_INFORMATION_MARK " ";

		if (relay_toogle_state() == true) {
			reply += TG_MSG_RELAY_IS_NOW_ON "\n";
		} else {
			reply += TG_MSG_RELAY_IS_NOW_OFF "\n";
		}

	} else {
		reply = EMOJI_INFORMATION_MARK " " TG_MSG_RELAY_CANT_BE_CTRL "\n\n";
	}

	return reply;
}
#endif

// =====================
// FUNCTIONS
// =====================

/**
 * @brief Inititalize the telegram module
 */
void telegram_init(void)
{
	struct telegram_cmd_t * pCmd;

	nextCheckTick = 0;

	// This is the simplest way of getting this working
  	// if you are passing sensitive information, or controlling
  	// something important, please either use certStore or at
  	// least client.setFingerPrint
	// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/issues/104#issuecomment-485255312
	wiFiClientSecure.setInsecure();

	// Define which command are available
	pCmd = &telegramCmds[telegramCmdsCount++];
	strncpy(pCmd->text, "/status", TG_CMD_TEXT_LEN);
	strncpy(pCmd->desc, TG_MSG_CMD_STATUS, TG_CMD_DESC_LEN);
	pCmd->callback = &telegram_cmd_status;

	pCmd = &telegramCmds[telegramCmdsCount++];
	strncpy(pCmd->text, "/start", TG_CMD_TEXT_LEN);
	strncpy(pCmd->desc, TG_MSG_CMD_START, TG_CMD_DESC_LEN);
	pCmd->callback = &telegram_cmd_start;

	pCmd = &telegramCmds[telegramCmdsCount++];
	strncpy(pCmd->text, "/stop", TG_CMD_TEXT_LEN);
	strncpy(pCmd->desc, TG_MSG_CMD_STOP, TG_CMD_DESC_LEN);
	pCmd->callback = &telegram_cmd_stop;

#if defined(MODULE_TEMPERATURE)
	pCmd = &telegramCmds[telegramCmdsCount++];
	strncpy(pCmd->text, "/sensors", TG_CMD_TEXT_LEN);
	strncpy(pCmd->desc, TG_MSG_CMD_SENSORS, TG_CMD_DESC_LEN);
	pCmd->callback = &telegram_cmd_sensors;
#endif

#if defined(MODULE_RELAY)
	pCmd = &telegramCmds[telegramCmdsCount++];
	strncpy(pCmd->text, "/relay", TG_CMD_TEXT_LEN);
	strncpy(pCmd->desc, TG_MSG_CMD_RELAY, TG_CMD_DESC_LEN);
	pCmd->callback = &telegram_cmd_relay;
#endif

	// Build keyboardJson
	// Model: "[[\"/start\", \"/stop\"], [\"/status\", \"/sensors\"]]";
	keyboardJson = "[";
	for (uint8_t i = 0; i < telegramCmdsCount; ++i) {
		// Get the shortcut
		pCmd = &telegramCmds[i];

		if ((i & 1) == 0) { // Left column
			// Add a coma between columns
			if (i > 0) {
				keyboardJson += ",";
			}

			keyboardJson += "[\"" + String(pCmd->text) + "\"";
		} else { // Right column
			keyboardJson += ",\"" + String(pCmd->text) + "\"]";
		}
	}
	// Close the last couple if count is odd
	if (telegramCmdsCount & 1) {
		keyboardJson += "]";
	}
	keyboardJson += "]";
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

/**
 * @brief Tell user about relay feedback when status changed
 */
void telegram_send_msg_relay_feedback(bool isOk)
{
	if (isOk) {
		telegram_send(EMOJI_GREEN_CHECK " " TG_MSG_GOOD_RELAY_FEEDBACK);
	} else {
		telegram_send(EMOJI_CROSS_MARK " " TG_MSG_BAD_RELAY_FEEDBACK);
	}
}

#endif